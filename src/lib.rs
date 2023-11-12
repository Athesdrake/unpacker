pub mod error;

mod movie_reader;
mod string_finder;
mod unpacker;
pub use movie_reader::MovieReader;
pub use unpacker::Unpacker;

#[allow(dead_code)]
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

#[cfg(test)]
mod tests {
    use reqwest;
    use std::fs::File;

    #[test]
    fn test_download() {
        let mut file = File::create("Transformice.swf").unwrap();
        reqwest::blocking::get("https://www.transformice.com/Transformice.swf")
            .unwrap()
            .copy_to(&mut file)
            .unwrap();
    }
}
