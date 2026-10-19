import cvAddon, { CvImage } from './bindings'
import fs from  'node:fs'
import path from 'node:path'


const input_image_path = path.join(__dirname, '..', 'assets', 'example_image.jpeg');
const output_dir_path = path.join(__dirname, '..', 'output');

console.log('input image path:', input_image_path);
console.log('output dir path:', output_dir_path);

const buffer = fs.readFileSync(input_image_path);
const image : CvImage= cvAddon.fromBuffer(buffer);

console.log('size of original image ', image.width(), image.height());

const resizedImage: CvImage = image.resize(320, 240);


console.log('size of resized image ', resizedImage.width(), resizedImage.height());

const out = resizedImage.toBuffer('.jpeg', 90);



// clean up.
image.release();
resizedImage.release();

fs.writeFileSync(path.join(output_dir_path, 'test_resized.jpeg'), out);
