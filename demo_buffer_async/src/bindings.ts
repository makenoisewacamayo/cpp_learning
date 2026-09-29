const addon = require('../build/Release/learning_cpp_buffer_async');

export interface BufferAsync {
    addAsync(a: number, b: number): Promise<number>;
    reverseBuffer(buffer: Buffer): Buffer;
    getName(): string;
}

export var BufferAsync: {
    new(name: string): BufferAsync
} = addon.BufferAsync;
