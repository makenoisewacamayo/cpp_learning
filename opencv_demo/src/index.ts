import cvAddon from './bindings'
import fs from  'node:fs'
import path from 'node:path'

function main() {
	const buffer = fs.readFileSync(path.join(__dirname, '..', 'assets', 'example_image.jpeg'));
    const image = cvAddon.fromBuffer(buffer);

    console.log('size', image.width(), image.height());
    const resizedImage = image.resize(320, 240);
    const out = resizedImage.toBuffer('.jpeg', 90);

    const outputDir = path.join(__dirname, '..', 'output');
    fs.mkdirSync(outputDir, { recursive: true });
    fs.writeFileSync(path.join(outputDir, 'test_resized.jpeg'), out);
}

main();
