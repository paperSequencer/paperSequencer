String webpage = "";


void append_page_header(){

webpage = "<!DOCTYPE html>";
webpage += "<html>";
webpage += "<head>";
webpage += "title>Paper Sequencer</title>";
webpage +=  "<style>";
webpage += "body {
      font-family: Arial, sans-serif;
      background-color: #f2f2f2;
      margin: 0;
      padding: 20px;
    }";
webpage += " h1 {
      color: #333333;
      text-align: center;
      margin-top: 30px;
    }";
webpage += "  h3 {
      color: #666666;
      text-align: center;
      margin-top: 30px;
    }";
webpage += " img {
      display: block;
      margin: 0 auto;
      max-width: 600px;
      height: auto;
      margin-bottom: 20px;
    }";
webpage += "p {
      color: #666666;
      text-align: center;
    }";
webpage += " .file-upload {
      text-align: center;
      margin-top: 20px;
    }";
webpage += ".file-upload input[type="file"] {
      display: none;
    }";
webpage += ".file-upload label {
      background-color: #4CAF50;
      color: #ffffff;
      padding: 10px 20px;
      font-size: 16px;
      cursor: pointer;
      border-radius: 5px;
    }";
webpage += " .file-upload label:hover {
      background-color: #45a049;
    }";
webpage += ".submit-button {
      display: block;
      margin: 0 auto;
      margin-top: 20px;
      padding: 15px 30px;
      font-size: 18px;
      background-color: #4CAF50;
      color: #ffffff;
      border: none;
      border-radius: 5px;
      cursor: pointer;
    }";
webpage +=".submit-button:hover {
      background-color: #45a049;
    }";
webpage += " </style>";
webpage += "</head>";
webpage += "<body>";
webpage += " <h1>Paper Sequencer - Song Selection</h1>";
webpage += "  <h3>The Paper Sequencer is an innovative project comprising of five synchronized disks that produce captivating sounds upon encountering the color black.</h3>";
webpage += "<audio src="bsound.mp3" autoplay loop></audio>";
webpage += "<img src="pic1.jpeg" alt="Paper Sequencer" />";
webpage += "<p>Please upload a WAV file for each of the five disks:</p>";
webpage += "<div class="file-upload">";
webpage += "<input type="file" id="disk1File" accept=".wav">";
webpage += " <label for="disk1File">Choose File for Disk 1</label>";
webpage += " </div>";
webpage += "<br>";
webpage += "<div class="file-upload">";
webpage += " <input type="file" id="disk2File" accept=".wav">";
webpage += "<label for="disk2File">Choose File for Disk 2</label>";
webpage += "</div>";
webpage += "<br>";
webpage += "<div class="file-upload">";
webpage +=  "<input type="file" id="disk3File" accept=".wav">";
webpage += " <label for="disk3File">Choose File for Disk 3</label>";
webpage += "</div>";
webpage += "<br>";
webpage += "<div class="file-upload">";
webpage += "<input type="file" id="disk4File" accept=".wav">";
webpage += "<label for="disk4File">Choose File for Disk 4</label>";
webpage += "</div>";
webpage += "<br>";
webpage += "<div class="file-upload">";
webpage += "<input type="file" id="disk5File" accept=".wav">";
webpage += "<label for="disk5File">Choose File for Disk 5</label>";
webpage += "</div>";
webpage += "<br><br>";
webpage += "<button class="submit-button" onclick="submitSongs()">Submit</button>";
webpage += "<script>";
webpage += "function submitSongs() {
      var disk1File = document.getElementById('disk1File').files[0];
      var disk2File = document.getElementById('disk2File').files[0];
      var disk3File = document.getElementById('disk3File').files[0];
      var disk4File = document.getElementById('disk4File').files[0];
      var disk5File = document.getElementById('disk5File').files[0];
  // You can perform further actions with the selected song files, such as storing them or displaying a confirmation message
  
  if (disk1File && disk2File && disk3File && disk4File && disk5File) {
    alert("Songs uploaded successfully!");
  } else {
    alert("Please select a WAV file for each disk.");
  }
}";
webpage += "</script>";
webpage += "</body>";
webpage += "</html>";



}
