use rabc::abc::parser::{InsIterator, Op, OpCode};
use std::collections::HashMap;

const STRING_SEQUENCE: &[OpCode] = &[OpCode::GetLocal0, OpCode::CallProperty];

pub struct StringFinder<'a> {
    pub prog: &'a mut InsIterator<'a>,
    methods: &'a HashMap<u32, u8>,
}

impl<'a> StringFinder<'a> {
    pub fn new(prog: &'a mut InsIterator<'a>, methods: &'a HashMap<u32, u8>) -> Self {
        Self { prog, methods }
    }

    pub fn is_add_string(&self) -> bool {
        self.prog.is(OpCode::Add) || self.prog.is_sequence(STRING_SEQUENCE)
    }

    pub fn match_target(&mut self, target: &str) -> bool {
        for target_byte in target.bytes() {
            match self.next_char() {
                Some(b) if self.methods[&b] == target_byte => {
                    continue;
                }
                _ => {}
            }
            self.skip_string();
            return false;
        }
        true
    }

    pub fn next_string(&mut self) -> bool {
        !self.prog.skip_until_seq(STRING_SEQUENCE).is_none()
    }
    fn skip_string(&mut self) {
        while self.is_add_string() {
            if self.prog.is(OpCode::Add) {
                self.prog.next();
            }
            self.prog.next();
        }
    }

    fn next_char(&mut self) -> Option<u32> {
        if !self.is_add_string() {
            return None;
        }

        while self.prog.is(OpCode::Add) {
            self.prog.next();
        }
        self.prog.next();

        let mut prop = None;
        if let Op::CallProperty(op) = &self.prog.get().op {
            prop = Some(op.property)
        } else {
            return prop;
        };
        self.prog.next().and(prop)
    }

    pub fn build(&mut self) -> Vec<u8> {
        let mut str = Vec::new();
        if !self.prog.is_sequence(STRING_SEQUENCE) {
            self.next_string();
        }

        while let Some(idx) = self.next_char() {
            if let Some(byte) = self.methods.get(&idx) {
                str.push(*byte);
            }
        }
        str
    }
}
