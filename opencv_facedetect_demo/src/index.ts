import cvAddon, { CvImage, FaceDetector } from './bindings';
import fs from 'node:fs';
import path from 'node:path';

const inputImagePath = path.join(
    __dirname, '..', 'assets', 'example_image.jpeg'
);
const outputDirPath = path.join(__dirname, '..', 'output');

const faceCascadePath = path.join(
    __dirname, '..', 'assets', 'haarcascade_frontalface_alt.xml'
);
const eyeCascadePath = path.join(
    __dirname, '..', 'assets', 'haarcascade_eye_tree_eyeglasses.xml'
);

const buffer = fs.readFileSync(inputImagePath);
const image: CvImage = cvAddon.fromBuffer(buffer);

console.log('Original image size:', image.width(), image.height());

const detector = new FaceDetector(
    faceCascadePath,
    eyeCascadePath,
    { scale: 1.3, tryFlip: false }
);
console.log('*************************************');
console.log('Detection result: ');
console.log('*************************************');

console.dir(detector.detect(image), { depth: null });

const result = detector.detectWithImage(image);
fs.writeFileSync(path.join(outputDirPath, 'test_face_detect.jpeg'), result.image);

image.release();
