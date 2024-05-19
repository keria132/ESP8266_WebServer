const getBetterLocation = async () => {
  const response = await fetch("http://ip-api.com/json/");

  if (response.ok) {
    const location = await response.json();

    return location.lat + "," + location.lon;
  }
}

const getWetherData = async () => {
  if(!document.querySelector(".mainWidget")){
    return;
  }
  const locationString = await getBetterLocation();
  const response = await fetch(`http://api.weatherapi.com/v1/current.json?key=44877cb0280f41d1a84130248241604&q=${locationString}&aqi=no`);

  if (response.ok) {
    const json = await response.json();
    console.log(json);
    displayWeatherWidget(json.current, json.location);
  };
}

const displayWeatherWidget = (weather, location) => {
  const weatherWidget = document.querySelector(".mainWidget");
  const loader = document.querySelector(".loader");

  weatherWidget.innerHTML = `
    <!--<div class="mainWidget__icon">⛅</div>-->
    <div class="mainWidget__icon">
      <img src="${weather.condition.icon}">
    </div>
    <h2 class="mainWidget__weather">${weather.condition.text}</h2>
    <small class="mainWidget__location">${location.region + ", " + location.country}</small>
    <p class="mainWidget__temperature">${weather.temp_c + "°"}</p>
    <span class="mainWidget__details">
      <p>${weather.feelslike_c + "°"}</p>
      <small>Sensible</small>
    </span>
    <span class="mainWidget__details">
      <p>${weather.precip_mm + "mm"}</p>
      <small>Precipitation</small>
    </span>
    <span class="mainWidget__details">
      <p>${weather.humidity + "%"}</p>
      <small>Humidity</small>
    </span>
    <span class="mainWidget__details">
      <p>${weather.wind_kph + " km/h"}</p>
      <small>Wind</small>
    </span>
  `

  weatherWidget.classList.toggle("mainWidget_loading");
  loader.classList.toggle("loader_loaded");
}

getWetherData();