use std::fmt;

#[derive(Clone, Copy, Debug, PartialEq, Eq, clap::ValueEnum)]
pub enum InitiumArch {
    #[value(name = "x86_64")]
    X86_64,
}

impl InitiumArch {
    fn as_str(self) -> &'static str {
        match self {
            InitiumArch::X86_64 => "x86_64",
        }
    }

    pub fn as_triple(self) -> &'static str {
        match self {
            InitiumArch::X86_64 => "x86_64-unknown-uefi",
        }
    }
}

impl Default for InitiumArch {
    fn default() -> Self {
        Self::X86_64
    }
}

impl fmt::Display for InitiumArch {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.as_str())
    }
}
