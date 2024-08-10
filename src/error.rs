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
    #[error(transparent)]
    RabcError(#[from] rabc::error::RabcError),
    #[error("utf8 error: {0}")]
    Utf8Error(#[from] std::string::FromUtf8Error),

    #[error("Index error in keymap: {0}")]
    KeymapIndexError(u8),
    #[error(transparent)]
    Missing(#[from] MissingError),
}
