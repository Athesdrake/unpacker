use crate::error::Result;
use rabc::{Movie, StreamReader};

pub trait MovieReader
where
    Self: Sized,
{
    fn from_buffer(buffer: Vec<u8>) -> Result<Self>;
    fn from_url(url: &str) -> Result<Self>;
    fn from_file(path: &str) -> Result<Self>;
}

impl MovieReader for Movie {
    fn from_buffer(buffer: Vec<u8>) -> Result<Self> {
        Ok(Self::read(&mut StreamReader::new(buffer))?)
    }

    fn from_url(url: &str) -> Result<Self> {
        let mut buffer = Vec::new();
        reqwest::blocking::get(url)
            .unwrap()
            .copy_to(&mut buffer)
            .unwrap();

        Self::from_buffer(buffer)
    }

    fn from_file(path: &str) -> Result<Self> {
        let buffer = std::fs::read(path)?;
        Self::from_buffer(buffer)
    }
}
