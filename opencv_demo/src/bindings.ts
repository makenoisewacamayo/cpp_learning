const addon = require('../build/Release/learning_cpp_opencv_demo');

export interface CvImage {
    toBuffer(ext?: string, quality?: number): Buffer;
    resize(width: number, height: number): CvImage;
    resizeAsync(width: number, height: number): Promise<CvImage>;
    width(): number;
    height(): number;
    release(): void;
}

export interface CvImageAddon {
    CvImage: { readonly prototype: CvImage };
    fromBuffer(data: Buffer): CvImage;
}

const cvAddon = addon as CvImageAddon;

export const CvImage = cvAddon.CvImage;
export const fromBuffer: (data: Buffer) => CvImage = cvAddon.fromBuffer;

export default cvAddon;
