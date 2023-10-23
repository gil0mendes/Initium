use std::ops::Deref;

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

#[derive(Debug, Subcommand)]
pub enum Action {
    Build(BuildOpt),
}

/// Developer utility for running various tasks.
#[derive(Debug, Parser)]
pub struct Opt {
    #[clap(subcommand)]
    pub action: Action,
}
