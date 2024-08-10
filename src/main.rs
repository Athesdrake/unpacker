use clap::Parser;
use rabc::Movie;
use std::{fs::File, time::Instant};
use unpacker::{error::Result, MovieReader, Unpacker};

/// Unpack Transformice SWF file
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// Increase output verbosity. Verbose messages go to stderr.
    #[arg(long, short = 'v', action = clap::ArgAction::Count, default_value_t = 0)]
    verbose: u8,

    /// The file url to unpack. Can be a file from the filesystem or an url to download.
    #[arg(short = 'i', long = None, default_value = "https://www.transformice.com/Transformice.swf")]
    input: String,

    /// The output file.
    #[arg()]
    output: String,
}

fn main() -> Result<()> {
    let mut timings = Vec::new();
    let args = Args::parse();
    let is_url = args.input.starts_with("http://") || args.input.starts_with("https://");
    stderrlog::new()
        .module(module_path!())
        .verbosity(match args.verbose {
            0 => log::Level::Warn,
            1 => log::Level::Info,
            _ => log::Level::Debug,
        })
        .init()
        .unwrap();

    log::debug!("input: {} url: {}", args.input, is_url);
    log::debug!("output: {}", args.output);

    let boot = Instant::now();
    let movie = if is_url {
        log::info!("Downloading file {}", args.input);
        Movie::from_url(&args.input)?
    } else {
        log::info!("Reading file {}", args.input);
        Movie::from_file(&args.input)?
    };
    timings.push(("Reading file", boot.elapsed()));
    let mut unp = Unpacker::new(&movie)?;

    log::info!("Resolving order");
    unp.resolve_order()?;
    timings.push(("Resolving order", boot.elapsed()));
    log::info!("Order: {:?}", unp.order);
    if unp.order.is_empty() {
        log::error!("Unable to resolve binaries order. Is it already unpacked?");
        return Ok(());
    }

    log::info!("Resolving binaries");
    unp.resolve_binaries()?;
    timings.push(("Resolving binaries", boot.elapsed()));

    log::info!("Writing to file {}", args.output);
    let mut file = File::create(args.output)?;
    if let Some(missing) = unp.write_binaries(&mut file)? {
        log::error!("Unable to find binary with name: {}", missing);
        return Ok(());
    }
    timings.push(("Writing file", boot.elapsed()));

    // Display stats
    if log::log_enabled!(log::Level::Debug) {
        log::debug!("Timing stats:");
        let mut last = boot;
        for (name, point) in timings {
            let took = boot + point - last;
            log::debug!(" - {name}: {took:.2?}");
            last = boot + point;
        }
        log::debug!("Total: {:.2?}", boot.elapsed());
    }
    Ok(())
}
