use anyhow::Result;
use cargo::{Cargo, CargoAction, Package};
use clap::Parser;
use opt::BuildOpt;
use util::run_cmd;

use crate::opt::Opt;

mod arch;
mod cargo;
mod opt;
mod util;

fn build(opt: &BuildOpt) -> Result<()> {
    let cargo = Cargo {
        action: CargoAction::Build,
        packages: Package::all_except_xtask(),
        release: opt.build_mode.release,
        target: Some(*opt.target),
        warning_as_error: false,
    };

    run_cmd(cargo.command()?)
}

fn main() -> Result<()> {
    let opt = Opt::parse();

    match &opt.action {
        opt::Action::Build(build_opt) => build(build_opt),
    }
}
