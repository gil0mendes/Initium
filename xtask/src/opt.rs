use std::{ops::Deref, path::PathBuf};

use clap::{Parser, Subcommand};

use crate::arch::InitiumArch;

/// Target option
#[derive(Debug, Parser)]
pub struct TargetOpt {
    #[clap(long, action, default_value_t)]
    pub target: InitiumArch,
}

impl Deref for TargetOpt {
    type Target = InitiumArch;

    fn deref(&self) -> &Self::Target {
        &self.target
    }
}

#[derive(Debug, Parser)]
pub struct BuildModeOpt {
    #[clap(long, action)]
    pub release: bool,
}

#[derive(Debug, Parser)]
pub struct BuildOpt {
    #[clap(flatten)]
    pub target: TargetOpt,

    #[clap(flatten)]
    pub build_mode: BuildModeOpt,
}

/// Options for the qemu command
#[derive(Debug, Parser)]
pub struct QemuOpt {
    #[clap(flatten)]
    pub target: TargetOpt,

    #[clap(flatten)]
    pub build_mode: BuildModeOpt,

    /// Disable hardware accelerated virtualization support in QEMU.
    #[clap(long, action)]
    pub disable_kvm: bool,

    /// Path of an OVMF code file.
    #[clap(long, action)]
    pub ovmf_code: Option<PathBuf>,

    /// Path of an OVMF vars file.
    #[clap(long, action)]
    pub ovmf_vars: Option<PathBuf>,
}

/// Possible actions to execute
#[derive(Debug, Subcommand)]
pub enum Action {
    Build(BuildOpt),
    Run(QemuOpt),
}

/// Developer utility for running various tasks.
#[derive(Debug, Parser)]
pub struct Opt {
    #[clap(subcommand)]
    pub action: Action,
}
