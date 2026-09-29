import { BufferAsync } from "./bindings";

async function main() {
	const obj = new BufferAsync("World");
    console.log("obj.getName(): ", obj.getName());
    const result = await obj.addAsync(5, 10);
    console.log("await obj.addAsync(5, 10) : ", result);
    const buffer = await obj.reverseBuffer(Buffer.from("Hello"));
    console.log("await obj.reverseBuffer(Buffer.from('Hello')): ", buffer.toString("utf8"));
    console.log("Hello from TypeScript!");
}   

main();
