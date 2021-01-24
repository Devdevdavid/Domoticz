/**
 * @brief Set error message in box id=status_msg
 */
function set_message(type, msg)
{
  var statusMsg = document.getElementById('status_msg');
  statusMsg.innerHTML = msg;

  if (type == 'hide') {
    statusMsg.style.display = "none";
  } else {
    statusMsg.style.display = "block";
    if (type == 'error') {
      statusMsg.classList.add("message_box_error");
      statusMsg.classList.remove("message_box_success");
    } else if (type == 'ok') {
      statusMsg.classList.remove("message_box_error");
      statusMsg.classList.add("message_box_success");
    }
  }
}

/**
 * Send params to url and call callback when response is ok
 */
function cgi_request(url, params, callback, onErrorCallback)
{
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4) {
      if (this.status == 200) {
        callback(xhttp.responseText);
      } else {
        if (onErrorCallback) {
          onErrorCallback();
        }
      }
    }
  };
  xhttp.open('GET', url + '?' + params, true);
  xhttp.send();
}