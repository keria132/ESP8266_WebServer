console.log("hello");

const getWetherAPI = async () => {
  let response = await fetch("/getWeatherData");
  if (response.ok) {
    let json = await response.json();
  } else {
    console.log("HTTP error! " + response.status)
  }
}