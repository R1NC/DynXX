function jStrToWasm(jstr) {
  var len = jstr.length + 1;
  var ptr = Module._malloc(len);
  stringToUTF8(jstr, ptr, len);
  return ptr;
}

function jStrFromWasm(ptr) {
  if (ptr === 0) {
    return "";
  }
  var len = 0;
  while (Module.HEAPU8[ptr + len] !== 0) {
    len++;
  }
  return Module.UTF8ToString(ptr, len);
}

function jNumArrToWasm(jarr) {
  if (!jarr || jarr.length === 0) {
    return 0;
  }
  var len = jarr.length + 1;
  var ptr = Module._malloc(len);
  Module.HEAPU8.set(new Uint8Array(jarr), ptr);
  return ptr;
}

function jNumArrFromWasm(ptr, len) {
  if (ptr === 0 || len === 0) {
    return [];
  }
  var heap = new Uint8Array(Module.HEAPU8.buffer, ptr, len);
  return Array.from(heap);
}

function readOutLen(outLenPtr) {
  if (outLenPtr === 0) {
    return 0;
  }
  var outLenLow = Module.HEAPU32[outLenPtr >> 2];
  var outLenHigh = Module.HEAPU32[(outLenPtr >> 2) + 1];
  return outLenLow + outLenHigh * 0x100000000;
}
