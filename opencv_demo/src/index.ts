import cvAddon, { CvImage } from './bindings'
import fs from  'node:fs'
import path from 'node:path'


const input_image_path = path.join(__dirname, '..', 'assets', 'example_image.jpeg');
const output_dir_path = path.join(__dirname, '..', 'output');

const buffer = fs.readFileSync(input_image_path);
const image : CvImage= cvAddon.fromBuffer(buffer);

console.log('size', image.width(), image.height());

const resizedImage: CvImage = image.resize(320, 240);
const out = resizedImage.toBuffer('.jpeg', 90);

fs.writeFileSync(path.join(output_dir_path, 'test_resized.jpeg'), out);
