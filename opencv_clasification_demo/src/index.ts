import { Classifier, fromBuffer } from './bindings';
import fs from 'node:fs';
import path from 'node:path';

const googlenetModelPath = path.join(
    __dirname, '..', 'assets', 'dnn/da39a3ee5e6b4b0d3255bfef95601890afd80709/googlenet-8.onnx'
);

const classificationClassesPath = path.join(
    __dirname, '..', 'assets', 'classification_classes_ILSVRC2012.txt'
);

const exampleImagePath = path.join(
    __dirname, '..', 'assets', 'test_example.jpg'
);

function main() {
    
    const classifier = new Classifier(googlenetModelPath, classificationClassesPath, {
        width: 224,
        height: 224,
        topK: 5,
        swapRB: false,
        crop: false,
    });

    const image = fromBuffer(fs.readFileSync(exampleImagePath));
    console.log('image size:', image.width(), 'x', image.height());

    const { predictions, image: annotated } = classifier.classify(image);

    console.log('Top predictions:');
    for (const p of predictions) {
        console.log(`  [${p.classId}] ${p.label}: ${(p.confidence).toFixed(4)}`);
    }

    const outputDir = path.join(__dirname, '..', 'output');
    fs.mkdirSync(outputDir, { recursive: true });
    const outPath = path.join(outputDir, 'annotated.jpg');
    fs.writeFileSync(outPath, annotated);
    console.log('Annotated image written to', outPath);

    image.release();
}

main();