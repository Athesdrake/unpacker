use crate::error::Result;
use rabc::{Movie, StreamReader};
use std::{fs::File, io::Read};

pub trait MovieReader
where
    Self: Sized,
{
    fn from_buffer(buffer: Vec<u8>) -> Result<Self>;
    fn from_url(url: &String) -> Result<Self>;
    fn from_file(file: File) -> Result<Self>;
}

impl MovieReader for Movie {
    fn from_buffer(buffer: Vec<u8>) -> Result<Self> {
        Ok(Movie::read(&mut StreamReader::new(buffer))?)
    }

    fn from_url(url: &String) -> Result<Self> {
        let mut buffer = Vec::new();
        reqwest::blocking::get(url)
            .unwrap()
            .copy_to(&mut buffer)
            .unwrap();

        Self::from_buffer(buffer)
    }

    fn from_file(mut file: File) -> Result<Self> {
        let mut buffer = Vec::with_capacity(file.metadata()?.len() as usize);
        file.read_to_end(&mut buffer)?;

        Self::from_buffer(buffer)
    }
}
