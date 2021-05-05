function init(){
	document.getElementById('fileInput').addEventListener('change', handleFileSelect, false);
}

function handleFileSelect(event){
	const reader = new FileReader()
	reader.onload = handleFileLoad;
	reader.readAsText(event.target.files[0])
}

function handleFileLoad(event){
	var level = JSON.parse(event.target.result);
	var layer = level.layers.find((layer) => layer.name=="Background");
	buildData(layer.data);
	
	
}

function buildData(data) {
	var buffer= new ArrayBuffer(864);
	var byteArray = new Uint8Array(buffer);
	
	byteArray[0] = 0x4E;
	byteArray[1] = 0x42;
	byteArray[2] = 0x4C;
	byteArray[3] = 0x30;
	byteArray[4] = 0x30;
	byteArray[5] = 0x30;
	byteArray[6] = 0x00;
	
	for (var i = 0; i < 23 * 15; i++) {
		byteArray[i+7] = data[i] - 1;
	}
	
	var base64String = btoa(String.fromCharCode.apply(null, byteArray));
	document.getElementById("fileDownload").href="data:application/octet-stream;base64," + base64String;
}