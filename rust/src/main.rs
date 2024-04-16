use std::io::{Error, ErrorKind, Read, Write};
use std::sync::Mutex;
use std::thread;
use std::time::{Duration, Instant};

use websockets::{connect, Connection};
use brotli2::BrotliEncoder;
use serde_json::Value;

const WEBSOCKET_URL: &str = "ws://localhost:9002";
const SERIAL_PORT: &str = "COM5";
const BAUD_RATE: u32 = 460800;

fn main() {
    let response_data: Mutex<Vec<u8>> = Mutex::new(vec![0; 38 * 49 * 3]);

    // Start serial sender thread
    let serial_thread = thread::spawn(move || {
        let mut serial = serial::open(SERIAL_PORT, BAUD_RATE).unwrap();
        serial.write_all(b"g").unwrap();

        loop {
            let start_time = Instant::now();

            let mut rgb_values = response_data.lock().unwrap().clone();
            for value in &mut rgb_values {
                *value = (*value * 255 / 215) & 0b1111_1110;
            }
            drop(response_data.lock().unwrap());

            let rgb_values = rgb_values.chunks(3)
                .map(|chunk| [chunk[1], chunk[0], chunk[2]]) // Swap green and blue
                .collect::<Vec<_>>();

            let image = ndarray::Array3::from_shape_vec((49, 38, 3), rgb_values).unwrap();
            let mut image_data = image.as_slice().to_owned();

            // Flip every other row
            for row in (0..38).step_by(5) {
                image_data.swap(row * 49, row * 49 + 48);
                for col in 1..(48 / 2 + 1) {
                    image_data.swap(row * 49 + col, row * 49 + 48 - col);
                }
            }

            let mut encoder = BrotliEncoder::new();
            let mut compressed_data: Vec<u8> = Vec::new();
            encoder.write_all(&image_data, &mut compressed_data).unwrap();
            encoder.finish(&mut compressed_data).unwrap();

            let message_len = compressed_data.len().to_be_bytes();
            let message = [&message_len, &compressed_data].concat();

            serial.read(mut [0]).unwrap(); // Read a byte (maybe for sync)
            serial.write_all(&message).unwrap();

            let elapsed = start_time.elapsed().as_secs_f32();
            println!(
                "{:.3f}s {:.3f} FPS {:.3f} Mb/s {} B",
                elapsed,
                1.0 / elapsed,
                message.len() as f32 / 1024.0 / 1024.0 / elapsed * 8.0,
                compressed_data.len()
            );
        }
    });

    // Start websocket handler
    let task = async {
        let mut connection = connect(WEBSOCKET_URL).await.unwrap();

        println!("connected");

        loop {
            connection.send(b"{\"id\":0,\"module\":\"drs\",\"function\":\"tapeled_get\",\"params\":[]}").await?;

            let message = connection.recv().await.unwrap();
            let data: Value = serde_json::from_slice(&message[..message.len() - 1]).unwrap();
            let response_data_slice = data["data"][0].as_array().unwrap();

            *response_data.lock().unwrap() = response_data_slice
                .iter()
                .map(|v| v.as_u64().unwrap() as u8)
                .collect();
        }
    };

    let _ = tokio::runtime::Runtime::new().unwrap().block_on(task);

    serial_thread.join().unwrap();
}