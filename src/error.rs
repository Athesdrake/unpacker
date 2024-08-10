use std::io;
use thiserror::Error;

pub type Result<T> = std::result::Result<T, UnpackerError>;

#[derive(Debug, Error)]
pub enum MissingError {
    #[error("This movie file does not have a frame1")]
    MissingFrame1,
    #[error("Cannot resolve methods: keymap was not found.")]
    MissingKeymap,
    #[error("Cannot resolve order: construct_super was not found.")]
    MissingSuper,
}

#[derive(Debug, Error)]
pub enum UnpackerError {
    #[error("IO error: {0}")]
    IoError(#[from] io::Error),
    #[error("RABC error: {0:?}")]
    RabcError(rabc::error::Error),
    #[error("utf8 error: {0}")]
    Utf8Error(#[from] std::string::FromUtf8Error),

    #[error("Index error in keymap: {0}")]
    KeymapIndexError(u8),
    #[error(transparent)]
    Missing(#[from] MissingError),
}

impl From<rabc::error::Error> for UnpackerError {
    fn from(error: rabc::error::Error) -> Self {
        match error {
            rabc::error::Error::IoError(error) => Self::IoError(error),
            error => Self::RabcError(error),
        }
    }
}
