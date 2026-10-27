const addon = require('../build/Release/learning_cpp_opencv_classifier_demo');

export interface CvImage {
    toBuffer(ext?: string, quality?: number): Buffer;
    resize(width: number, height: number): CvImage;
    resizeAsync(width: number, height: number): Promise<CvImage>;
    width(): number;
    height(): number;
    release(): void;
}

export interface ClassifierOptions {
    width?: number;
    height?: number;
    topK?: number;
    scale?: number;
    swapRB?: boolean;
    crop?: boolean;
}

export interface Prediction {
    classId: number;
    label: string;
    confidence: number;
}

export interface ClassificationResult {
    predictions: Prediction[];
    image: Buffer;
}

export interface Classifier {
    classify(image: CvImage): ClassificationResult;
}

export interface ClassifierConstructor {
    /**
     * @param modelPath Path to an ONNX model
     * @param labelsPath Optional path to a text file with one label per line
     * @param options Optional preprocessing/inference options
     */
    new (modelPath: string, labelsPath?: string, options?: ClassifierOptions): Classifier;
}

export const CvImage: { readonly prototype: CvImage } = addon.CvImage;
export const fromBuffer: (data: Buffer) => CvImage = addon.fromBuffer;
export const Classifier: ClassifierConstructor = addon.Classifier;

export default addon;