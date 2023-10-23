use std::process::Command;

use anyhow::{Ok, Result};

use crate::arch::InitiumArch;

#[derive(Clone, Copy, Debug)]
pub enum Package {
    Initium,
    Xtask,
}

impl Package {
    pub fn all_except_xtask() -> Vec<Package> {
        vec![Self::Initium]
    }
}

#[derive(Clone, Copy, Debug)]
pub enum CargoAction {
    Build,
}

#[derive(Debug)]
pub struct Cargo {
    pub action: CargoAction,
    pub packages: Vec<Package>,
    pub release: bool,
    pub target: Option<InitiumArch>,
    pub warning_as_error: bool,
}

impl Cargo {
    pub fn command(&self) -> Result<Command> {
        let mut cmd = Command::new("cargo");

        let action;
        let mut sub_action: Option<&str> = None;
        let mut extra_args: Vec<&str> = Vec::new();
        let mut tool_args: Vec<&str> = Vec::new();

        match self.action {
            CargoAction::Build => {
                action = "build";
            }
        };

        cmd.arg(action);
        if let Some(sub_action) = sub_action {
            cmd.arg(sub_action);
        }

        if self.release {
            cmd.arg("--release");
        }

        if let Some(target) = self.target {
            cmd.args(["--target", target.as_triple()]);
        }

        cmd.args(extra_args);

        if !tool_args.is_empty() {
            cmd.arg("--");
            cmd.args(tool_args);
        }

        Ok(cmd)
    }
}
