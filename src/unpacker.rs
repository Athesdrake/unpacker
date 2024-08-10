use crate::{
    error::{MissingError, Result, UnpackerError},
    string_finder::StringFinder,
};
use rabc::{
    abc::{
        parser::{InsIter, Instruction, Op, OpCode},
        Trait,
    },
    AbcFile, Movie,
};
use std::{collections::HashMap, io::Write};

#[non_exhaustive]
pub struct Unpacker<'a> {
    pub movie: &'a Movie,
    pub abc: &'a AbcFile,

    pub keymap: Option<String>,
    pub methods: HashMap<u32, u8>,
    pub order: Vec<String>,
    pub binaries: HashMap<String, &'a Vec<u8>>,
}

impl<'a> Unpacker<'a> {
    pub fn new(movie: &'a Movie) -> Result<Self> {
        Ok(Unpacker {
            movie,
            abc: &movie.frame1().ok_or(MissingError::MissingFrame1)?.abcfile,
            keymap: None,
            methods: HashMap::default(),
            order: Vec::default(),
            binaries: HashMap::default(),
        })
    }

    /// Unpack the movie to the given writer
    pub fn unpack(&mut self, writer: &mut dyn Write) -> Result<Option<&String>> {
        self.resolve_order()?;
        self.resolve_binaries()?;
        self.write_binaries(writer)
    }

    fn resolve_keymap(&mut self, instructions: &Vec<Instruction>) {
        let mut prog = instructions.iter_prog();
        while prog.has_next() && prog.skip_until(OpCode::PushString).is_some() {
            if let Op::PushString(op) = &prog.get().op {
                self.keymap = self.abc.cpool.strings.get(op.value as usize).cloned();
                break;
            }
        }
    }
    fn resolve_methods(&mut self) -> Result<()> {
        let keymap = self.keymap.clone().ok_or(MissingError::MissingKeymap)?;

        // Get all methods taking a ...rest argument
        // Those methods return a single character from the keymap
        for tr in &self.abc.classes[0].itraits {
            if let Trait::Method(tr) = tr {
                let Some(method) = self.abc.methods.get(tr.index as usize) else {
                    continue;
                };
                if method.need_rest() && method.max_stack == 2 {
                    let instructions = method.parse()?;
                    let mut prog = instructions.iter_prog();

                    // Get the returned character
                    if prog.skip_until(OpCode::PushByte).is_some() {
                        if let Op::PushByte(op) = &prog.get().op {
                            self.methods.insert(
                                tr.name,
                                *keymap
                                    .as_bytes()
                                    .get(op.value as usize)
                                    .ok_or_else(|| UnpackerError::KeymapIndexError(op.value))?,
                            );
                        }
                    }
                }
            }
        }
        Ok(())
    }
    pub fn resolve_order(&mut self) -> Result<()> {
        let class = self.abc.get_class(0)?;
        // Get the keymap from the cinit method
        // then resolve the methods return value
        self.resolve_keymap(&self.abc.get_method(class.cinit)?.parse()?);
        self.resolve_methods()?;

        let instructions = self.abc.get_method(class.iinit)?.parse()?;
        let mut prog = instructions.iter_prog();
        let mut finder = StringFinder::new(&mut prog, &self.methods);
        finder
            .prog
            .skip_until(OpCode::ConstructSuper)
            .ok_or(MissingError::MissingSuper)?;

        while finder.next_string() {
            if finder.match_target("writeBytes") {
                // The next string is the binary's name
                finder.next_string();
                self.order.push(String::from_utf8(finder.build())?);
            }
        }
        Ok(())
    }
    pub fn resolve_binaries(&mut self) -> Result<()> {
        for tag in self.movie.binaries() {
            if let Some(symbol) = self.movie.symbols.get(&tag.char_id) {
                // Remove the prefix
                if let Some(name) = symbol.split_once('_').map(|(_, s)| s) {
                    self.binaries.insert(name.to_owned(), &tag.data);
                }
            }
        }
        Ok(())
    }
    pub fn write_binaries(&self, out: &mut dyn Write) -> Result<Option<&String>> {
        for name in &self.order {
            match self.binaries.get(name) {
                Some(bin) => out.write_all(bin.as_slice())?,
                None => return Ok(Some(name)),
            }
        }
        Ok(None)
    }
}
