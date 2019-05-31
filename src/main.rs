use std::io::Read;
use std::io::Write;
use std::io::Seek;

fn main() -> std::io::Result<()> {
    let args: Vec<String> = std::env::args().collect();
    println!();

    if args.len() < 3 {
        println!("FNT converter v1.0");
        println!("Usage: {} [input] [output]", args[0]);
        std::process::exit(0);
    }

    let split = args[2].split(".");
    let v_name: Vec<&str> = split.collect();

    let mut input = std::fs::File::open(&args[1])?;

    let mut buf: [u8; 4] = [0; 4];
    let font_height: u32;
    let glyph_count: u32;
    let header_size: u32 = 48;
    let mut buf_size: u32 = header_size;

    input.read(&mut buf)?;
    font_height = (buf[0] as u32) | ((buf[1] as u32) << 8) | ((buf[2] as u32) << 16) | ((buf[3] as u32) << 24);

    input.read(&mut buf)?;
    glyph_count = (buf[0] as u32) | ((buf[1] as u32) << 8) | ((buf[2] as u32) << 16) | ((buf[3] as u32) << 24);

    println!("[FNT Info]");
    println!("- font_height: {}", font_height);
    println!("- glyph_count: {}", glyph_count);

    // Calculate total data size
    for _i in 0..glyph_count {
        input.read(&mut buf)?;
        input.read(&mut buf)?;
        input.read(&mut buf)?;
        let data_size = (buf[0] as u32) | ((buf[1] as u32) << 8) | ((buf[2] as u32) << 16) | ((buf[3] as u32) << 24);

        let mut data = vec![0; data_size as usize];
        input.read(&mut data)?;

        buf_size = buf_size + 32;
        buf_size = buf_size + data_size;
    }

    println!("Total buffer size: {}", buf_size);

    let mut output = std::fs::File::create(&args[2])?;

    write!(&mut output, "#include <stdint.h>\n\n")?;
    write!(&mut output, "uint8_t {}[{}] = {{\n", v_name[0], buf_size)?;

    // Write header
    write!(&mut output, "\t0x00, 0x00, 0x00, 0x00, // uint32_t id\n")?;
    write!(&mut output, "\t0x{:02x}, 0x{:02x}, 0x{:02x}, 0x{:02x}, // uint32_t font_height\n",
        (font_height & 0x000000ff),
        (font_height & 0x0000ff00) >> 8,
        (font_height & 0x00ff0000) >> 16,
        (font_height & 0xff000000) >> 24)?;
    write!(&mut output, "\t0x{:02x}, 0x{:02x}, 0x{:02x}, 0x{:02x}, // uint32_t glyph_count\n",
        (glyph_count & 0x000000ff),
        (glyph_count & 0x0000ff00) >> 8,
        (glyph_count & 0x00ff0000) >> 16,
        (glyph_count & 0xff000000) >> 24)?;
    write!(&mut output, "\t0x{:02x}, 0x{:02x}, 0x{:02x}, 0x{:02x}, // uint32_t header_size\n",
        (48u32 & 0x000000ff),
        (48u32 & 0x0000ff00) >> 8,
        (48u32 & 0x00ff0000) >> 16,
        (48u32 & 0xff000000) >> 24)?;

    for i in 0..8 {
        write!(&mut output, "\t0x00, 0x00, 0x00, 0x00, // int32_t reserved[{}]\n", i)?;
    }

    input.seek(std::io::SeekFrom::Start(8))?;

    for i in 0..glyph_count {
        input.read(&mut buf)?;
        let utf8_code = (buf[0] as u32) | ((buf[1] as u32) << 8) | ((buf[2] as u32) << 16) | ((buf[3] as u32) << 24);

        input.read(&mut buf)?;
        let font_width = (buf[0] as u32) | ((buf[1] as u32) << 8) | ((buf[2] as u32) << 16) | ((buf[3] as u32) << 24);

        input.read(&mut buf)?;
        let data_size = (buf[0] as u32) | ((buf[1] as u32) << 8) | ((buf[2] as u32) << 16) | ((buf[3] as u32) << 24);

        let mut data = vec![0; data_size as usize];
        input.read(&mut data)?;

        write!(&mut output, "\t0x{:02x}, 0x{:02x}, 0x{:02x}, 0x{:02x}, // uint32_t utf8_code\n",
            (utf8_code & 0x000000ff),
            (utf8_code & 0x0000ff00) >> 8,
            (utf8_code & 0x00ff0000) >> 16,
            (utf8_code & 0xff000000) >> 24)?;

        write!(&mut output, "\t0x{:02x}, 0x{:02x}, 0x{:02x}, 0x{:02x}, // uint32_t font_width\n",
            (font_width & 0x000000ff),
            (font_width & 0x0000ff00) >> 8,
            (font_width & 0x00ff0000) >> 16,
            (font_width & 0xff000000) >> 24)?;

        write!(&mut output, "\t0x20, 0x00, 0x00, 0x00, // uint32_t header_size\n")?;

        write!(&mut output, "\t0x{:02x}, 0x{:02x}, 0x{:02x}, 0x{:02x}, // uint32_t data_size\n",
            (data_size & 0x000000ff),
            (data_size & 0x0000ff00) >> 8,
            (data_size & 0x00ff0000) >> 16,
            (data_size & 0xff000000) >> 24)?;

        for j in 0..4 {
            write!(&mut output, "\t0x00, 0x00, 0x00, 0x00, // int32_t reserved[{}]\n", j)?;
        }

        write!(&mut output, "\t")?;
        for j in 0..data_size {
            write!(&mut output, "0x{:02x}, ", data[j as usize])?;
        }
        write!(&mut output, "\n")?;

        println!("[{:.2}%] {}/{} Done..", ((i + 1) as f32 / glyph_count as f32) * 100f32, (i + 1), glyph_count);
    }

    write!(&mut output, "}};\n\n")?;

    println!("\n{} created successfully!", &args[2]);

    Ok(())
}
