pub mod error;

mod movie;
mod string_finder;
mod unpacker;
pub use movie::MovieReader;
pub use unpacker::Unpacker;

#[allow(dead_code)]
pub const VERSION: &str = env!("CARGO_PKG_VERSION");
