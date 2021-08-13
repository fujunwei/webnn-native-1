const webnn = require('./lib/webnn');
// navigator is undefined in node.js, but defined in electron.js.
if (global.navigator === undefined) {
  global.navigator = {};
}
global.navigator.native_ml = webnn.ml;
global.native_MLContext = webnn.MLContext
global.native_MLGraphBuilder = webnn.MLGraphBuilder
global.native_MLGraph = webnn.MLGraph
global.native_MLOperand = webnn.MLOperand
global.chai = require('chai');
global.fs = require('fs');