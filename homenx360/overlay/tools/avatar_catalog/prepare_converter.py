#!/usr/bin/env python3
"""Add an offline head-channel export hook to a pinned avatar-aura checkout.

This writes our small hook into the caller's research checkout. No third-party
implementation is vendored here. Build the avatarextract CLI after running it.
"""
import argparse
from pathlib import Path
REVISION='2d2e4a4cfccd795b8ed3e14a19a942c83d298ad5'
HOOK=r'''
            // HomeNX360 offline catalog channel export. Keep each original mask
            // in head UV space; the Switch combines the selected masks/colors.
            if is_head && std::env::var_os("NXE_CATALOG_MAPS").is_some() {
                let channels = [("base", &recipe.base), ("shadow", &recipe.eyeshadow),
                    ("mouth", &recipe.mouth), ("eyes", &recipe.eye),
                    ("facial", &recipe.facial_hair), ("brows", &recipe.brow)];
                for (label, sampler) in channels {
                    let raw = MaterialRecipe { kind: ShaderKind::Body,
                        color: sampler.clone(), out_uv_layer: recipe.out_uv_layer,
                        ..Default::default() };
                    let image = crate::textures::bake_material(raw, &mesh.verts,
                        &mesh.indices, 256, (256,256), None);
                    if !write_png(&self.args.out_dir.join(format!("channel_{bi}_{label}.png")), &image) {
                        panic!("failed to export catalog channel");
                    }
                }
            }
'''
def prepare(path):
    import subprocess
    if subprocess.check_output(['git','rev-parse','HEAD'],cwd=path,text=True).strip()!=REVISION:
        raise SystemExit('Converter source revision differs from the reviewed revision')
    source=path/'crates/avatar-bake/src/bake.rs';text=source.read_text()
    anchor='            let decal_layer = recipe.decal.clone();'
    if 'HomeNX360 offline catalog channel export' not in text:
        assert text.count(anchor)==1
        source.write_text(text.replace(anchor,HOOK+'\n'+anchor))
    # Several genuine Marketplace costumes exceed the port's original 8,192
    # triangles per batch. Keep bounded limits consistent with the native bank.
    model=path/'crates/avatar-formats/src/model.rs';text=model.read_text()
    text=text.replace('b.triangle_count <= 8192, "triangle count above 8192"', 'b.triangle_count <= 32768, "triangle count above 32768"')
    text=text.replace('vertex_count <= 8192, "vertex count above 8192"', 'vertex_count <= 20000, "vertex count above 20000"')
    model.write_text(text)
    # A few signed Marketplace blocks retain up to 32 bytes of encoder padding
    # after their final partial LZX frame. Never interpret that padding as a
    # new compressed-block header; all real frame bounds remain checked.
    compression=path/'crates/avatar-formats/src/compression.rs';text=compression.read_text()
    marker='fn trim_frame_padding(input: &[u8]) -> &[u8]'
    if marker not in text:
        helper=r"""
fn trim_frame_padding(input: &[u8]) -> &[u8] {
    let mut offset=0usize;let mut output=0usize;
    while offset+BLOCK_HEADER_SIZE<=input.len() {
        let h=block_header(input,offset);
        if h.compressed_size>input.len()-offset-BLOCK_HEADER_SIZE || h.uncompressed_offset!=output || h.uncompressed_size>WINDOW_SIZE as usize { return input; }
        offset+=BLOCK_HEADER_SIZE+h.compressed_size;output+=h.uncompressed_size;
        if h.uncompressed_size<WINDOW_SIZE as usize {
            return if input.len()-offset<=32 { &input[..offset] } else { input };
        }
    }
    input
}
"""
        text+=helper
        text=text.replace('pub fn uncompressed_size(input: &[u8]) -> Result<usize> {','pub fn uncompressed_size(input: &[u8]) -> Result<usize> {\n    let input=trim_frame_padding(input);')
        text=text.replace('pub fn decompress_into(input: &[u8], output: &mut [u8]) -> Result<()> {','pub fn decompress_into(input: &[u8], output: &mut [u8]) -> Result<()> {\n    let input=trim_frame_padding(input);')
        compression.write_text(text)
    # The Windows desktop GUI is not part of this offline command-line build.
    workspace=path/'Cargo.toml';text=workspace.read_text()
    text=text.replace('"crates/avatar-aura", ', '').replace(', "crates/avatar-aura"','')
    text=text.replace('"crates/avatar-view", ', '').replace(', "crates/avatar-view"','')
    workspace.write_text(text)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('checkout',type=Path);prepare(p.parse_args().checkout)
