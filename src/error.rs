use std::{borrow, io};

pub type Result<T> = std::result::Result<T, Error>;

#[derive(Debug)]
pub enum Error {
    IoError(io::Error),
    RabcError(rabc::error::Error),
    Missing(borrow::Cow<'static, str>),
    IndexError(),
}

impl Error {
    #[inline]
    pub fn missing(message: impl Into<borrow::Cow<'static, str>>) -> Self {
        Self::Missing(message.into())
    }
}

impl From<io::Error> for Error {
    fn from(error: io::Error) -> Self {
        Self::IoError(error)
    }
}
impl From<rabc::error::Error> for Error {
    fn from(error: rabc::error::Error) -> Self {
        match error {
            rabc::error::Error::IoError(error) => Self::IoError(error),
            error => Self::RabcError(error),
        }
    }
}
impl From<std::string::FromUtf8Error> for Error {
    fn from(error: std::string::FromUtf8Error) -> Self {
        Self::RabcError(rabc::error::Error::FromUtf8Error(error))
    }
}
