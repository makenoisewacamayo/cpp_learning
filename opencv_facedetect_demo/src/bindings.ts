const addon = require('../build/Release/learning_cpp_opencv_facedetect_demo');

export interface CvImage {
    toBuffer(ext?: string, quality?: number): Buffer;
    resize(width: number, height: number): CvImage;
    resizeAsync(width: number, height: number): Promise<CvImage>;
    width(): number;
    height(): number;
    release(): void;
}

export interface FaceRect {
    x: number;
    y: number;
    width: number;
    height: number;
}

export interface FaceDetection extends FaceRect {
    eyes: FaceRect[];
}

export interface FaceDetectorOptions {
    scale?: number;
    tryFlip?: boolean;
}

export interface FaceDetectorInstance {
    detect(image: CvImage): FaceDetection[];
    detectWithImage(image: CvImage): { faces: FaceDetection[]; image: Buffer };
}

export interface FaceDetectorConstructor {
    new (
        faceCascadePath: string,
        nestedCascadePath?: string,
        options?: FaceDetectorOptions
    ): FaceDetectorInstance;
}

export interface CvImageAddon {
    CvImage: { readonly prototype: CvImage };
    fromBuffer(imageBuffer: Buffer): CvImage;
    FaceDetector: FaceDetectorConstructor;
}

const cvAddon = addon as CvImageAddon;

export const CvImage = cvAddon.CvImage;
export const fromBuffer = cvAddon.fromBuffer;
export const FaceDetector = cvAddon.FaceDetector;

export default cvAddon;