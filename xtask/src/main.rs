use anyhow::Result;
use cargo::{Cargo, CargoAction, Package, TargetType};
use clap::Parser;
use opt::{BuildOpt, QemuOpt};
use util::run_cmd;

use crate::opt::Opt;

mod arch;
mod cargo;
mod disk;
mod opt;
mod pipe;
mod platform;
mod qemu;
mod util;

fn build(opt: &BuildOpt) -> Result<()> {
    let cargo = Cargo {
        action: CargoAction::Build,
        packages: Package::all_except_xtask(),
        release: opt.build_mode.release,
        target: Some(*opt.target),
        warning_as_error: false,
        target_types: TargetType::Default,
    };

    run_cmd(cargo.command()?)
}

/// Run project on QEMU
fn run_vm(opt: &QemuOpt) -> Result<()> {
    qemu::run_qemu(*opt.target, opt)
}

fn main() -> Result<()> {
    let opt = Opt::parse();

    match &opt.action {
        opt::Action::Build(build_opt) => build(build_opt),
        opt::Action::Run(qemu_opt) => run_vm(qemu_opt),
    }
}
