use common::video::{ConsoleOut, FrameBuffer};

pub struct ConsoleOutManager<'a> {
    framebuffer: FrameBuffer<'a>,
}

impl ConsoleOutManager<'a> {
    pub fn new(framebuffer: FrameBuffer<'a>) -> Self {
        Self {
            framebuffer
        }
    }
}

impl ConsoleOut for ConsoleOutManager<'_> {
    fn clear(&self, x: u16, y: u16, width: u16, height: u16) {
        unimplemented!()
    }
}
