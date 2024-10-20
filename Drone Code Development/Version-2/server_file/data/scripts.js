document.addEventListener("DOMContentLoaded", function () {
    const startButton = document.getElementById('startButton');
    const calibrateButton = document.getElementById('calibrateButton');
    const statusElement = document.getElementById('status');
    const batteryLevel = document.getElementById('batteryLevel');
  
    // Handle Start Drone button click
    startButton.addEventListener('click', function () {
      console.log('Start Drone');
      statusElement.innerText = 'Drone Started'; // Update status
    });
  
    // Handle Calibrate Drone button click
    calibrateButton.addEventListener('click', function () {
      console.log('Calibrate Drone');
      statusElement.innerText = 'Calibrating...'; // Update status
      setTimeout(() => {
        statusElement.innerText = 'Calibration Successful'; // Simulate calibration success
      }, 2000);
    });
  
    // Simulate updating the battery level every 5 seconds
    setInterval(() => {
      batteryLevel.innerText = Math.floor(Math.random() * 100) + '%'; // Dummy battery level
    }, 5000);
  
    // Handle joystick movements (move the inner circle)
    const moveJoystick = document.getElementById('moveJoystick');
    const moveInnerCircle = document.querySelector('#moveJoystick .inner-circle');
  
    moveJoystick.addEventListener('mousemove', function (event) {
      const rect = moveJoystick.getBoundingClientRect();
      const x = event.clientX - rect.left - moveInnerCircle.offsetWidth / 2;
      const y = event.clientY - rect.top - moveInnerCircle.offsetHeight / 2;
  
      // Restrict movement within the joystick container
      const maxX = rect.width - moveInnerCircle.offsetWidth;
      const maxY = rect.height - moveInnerCircle.offsetHeight;
      moveInnerCircle.style.left = Math.max(0, Math.min(x, maxX)) + 'px';
      moveInnerCircle.style.top = Math.max(0, Math.min(y, maxY)) + 'px';
  
      console.log('Movement Joystick Moved:', x, y);
    });
  
    const altitudeJoystick = document.getElementById('altitudeJoystick');
    const altitudeInnerCircle = document.querySelector('#altitudeJoystick .inner-circle');
  
    altitudeJoystick.addEventListener('mousemove', function (event) {
      const rect = altitudeJoystick.getBoundingClientRect();
      const x = event.clientX - rect.left - altitudeInnerCircle.offsetWidth / 2;
      const y = event.clientY - rect.top - altitudeInnerCircle.offsetHeight / 2;
  
      // Restrict movement within the joystick container
      const maxX = rect.width - altitudeInnerCircle.offsetWidth;
      const maxY = rect.height - altitudeInnerCircle.offsetHeight;
      altitudeInnerCircle.style.left = Math.max(0, Math.min(x, maxX)) + 'px';
      altitudeInnerCircle.style.top = Math.max(0, Math.min(y, maxY)) + 'px';
  
      console.log('Altitude Joystick Moved:', x, y);
    });
  
    // Optional: Add touch support for mobile devices
    moveJoystick.addEventListener('touchmove', function (event) {
      const rect = moveJoystick.getBoundingClientRect();
      const x = event.touches[0].clientX - rect.left - moveInnerCircle.offsetWidth / 2;
      const y = event.touches[0].clientY - rect.top - moveInnerCircle.offsetHeight / 2;
  
      // Restrict movement within the joystick container
      const maxX = rect.width - moveInnerCircle.offsetWidth;
      const maxY = rect.height - moveInnerCircle.offsetHeight;
      moveInnerCircle.style.left = Math.max(0, Math.min(x, maxX)) + 'px';
      moveInnerCircle.style.top = Math.max(0, Math.min(y, maxY)) + 'px';
  
      console.log('Movement Joystick Touched:', x, y);
    });
  
    altitudeJoystick.addEventListener('touchmove', function (event) {
      const rect = altitudeJoystick.getBoundingClientRect();
      const x = event.touches[0].clientX - rect.left - altitudeInnerCircle.offsetWidth / 2;
      const y = event.touches[0].clientY - rect.top - altitudeInnerCircle.offsetHeight / 2;
  
      // Restrict movement within the joystick container
      const maxX = rect.width - altitudeInnerCircle.offsetWidth;
      const maxY = rect.height - altitudeInnerCircle.offsetHeight;
      altitudeInnerCircle.style.left = Math.max(0, Math.min(x, maxX)) + 'px';
      altitudeInnerCircle.style.top = Math.max(0, Math.min(y, maxY)) + 'px';
  
      console.log('Altitude Joystick Touched:', x, y);
    });
  });
  