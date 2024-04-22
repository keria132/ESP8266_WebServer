console.log("hello");

const getWetherAPI = async () => {
  let response = await fetch("/getWeatherData");
  if (response.ok) {
    let json = await response.json();
  } else {
    console.log("HTTP error! " + response.status)
  }
}

const weatherAPI = async () => {
  const url = "http://api.weatherapi.com/v1/current.json?key=44877cb0280f41d1a84130248241604&q=192.162.0.0"
  let response = await fetch(url, {
  });
  let data = await response.json();
  console.log(data)
  return response;
}

weatherAPI()