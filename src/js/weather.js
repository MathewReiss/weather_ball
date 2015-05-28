var latitude = 0, longitude = 0;

function HTTP_GET(){
	try{
		var req = new XMLHttpRequest();
		var url = "http://api.openweathermap.org/data/2.5/weather?lat="+latitude+"&lon="+longitude;
		req.open("GET", url, false);
		req.send();
		return req.responseText;
	}
	catch(err){
		console.error(err);
	}
}

function getWeather(){
	var response = HTTP_GET();
	var weather_data = JSON.parse(response);
	var status = weather_data.weather[0].id;

	if(status >= 200 && status < 300) status = "2"; //Thunderstorm
	else if(status >= 300 && status < 600) status = "1"; //Rain
	else if(status == 602 || status == 622) status = "7"; //Blizzard
	else if(status >= 600 && status < 700) status = "5"; //Snow
	else if(status >= 700 && status < 800) status = "6"; //Fog
	else if(status == 800) status = "0"; //Sunny
	else if(status == 801 || status == 802) status = "3"; //Partly Cloudy
	else if(status == 803 || status == 804) status = "4"; //Mostly Cloudy
	else status = "7"; //Empty - Default
	
	console.log("Weather: " + status);
	
	Pebble.sendAppMessage({"WEATHER" : status});
}

function showPosition(position){
	latitude = position.coords.latitude;
	longitude = position.coords.longitude;
}

Pebble.addEventListener("appmessage", function(e){
	getWeather();
});

Pebble.addEventListener("ready", function(e){
	if(navigator && navigator.geolocation){
		navigator.geolocation.watchPosition(showPosition);
	}
});