use rabc::Movie;
use unpacker::MovieReader;
use unpacker::error::Result;

pub trait  MovieFromUrl
where Self:Sized, Self: MovieReader{
    fn from_url(url: &str) -> Result<Self>; }

impl MovieFromUrl for Movie {

    fn from_url(url: &str) -> Result<Self> {
        let mut buffer = Vec::new();
        reqwest::blocking::get(url)
        .unwrap()
        .copy_to(&mut buffer)
        .unwrap();

    Self::from_buffer(buffer)
}
}