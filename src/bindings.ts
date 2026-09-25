const addon = require('../build/Release/learning_cpp');

export interface MyObject {
    greet(name: string): void;
    add(a: number, b: number): number;
}

export var MyObject: {
    new(name: string): MyObject
} = addon.MyObject;
