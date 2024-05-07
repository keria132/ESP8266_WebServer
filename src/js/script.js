console.log("hello");

// const getWetherAPI = async () => {
//   let response = await fetch("/getWeatherData");
//   if (response.ok) {
//     let json = await response.json();
//   } else {
//     console.log("HTTP error! " + response.status)
//   }
// }

const getWetherData = async () => {
  let response = await fetch("http://api.weatherapi.com/v1/current.json?key=44877cb0280f41d1a84130248241604&q=94.233.241.231&aqi=no")
  if (response.ok) {
    let json = await response.json();
    console.log(json);
    createWeatherWidget(json.current, json.location)
  }
}

const createWeatherWidget = (weather, location) => {
  const mainPageWidget = document.querySelector(".mainPageWidget");
  mainPageWidget.innerHTML = `
    <div class="mainPageWidget__icon">⛅</div>
    <!-- <img class="mainPageWidget__icon" src="./src/assets/partly__cloudy.svg" alt=""> -->
    <h2 class="mainPageWidget__weather">${weather.condition.text}</h2>
    <small class="mainPageWidget__location">${location.region + ", " + location.country}</small>
    <p class="mainPageWidget__temperature">${weather.temp_c + "°"}</p>
    <span class="mainPageWidget__details">
      <p>${weather.feelslike_c + "°"}</p>
      <small>Sensible</small>
    </span>
    <span class="mainPageWidget__details">
      <p>${weather.precip_mm + "mm"}</p>
      <small>Precipitation</small>
    </span>
    <span class="mainPageWidget__details">
      <p>${weather.humidity + "%"}</p>
      <small>Humidity</small>
    </span>
    <span class="mainPageWidget__details">
      <p>${weather.wind_kph + " km/h"}</p>
      <small>Wind</small>
    </span>
  `
}

getWetherData();