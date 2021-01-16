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
    } else {
      statusMsg.classList.remove("message_box_error");
      statusMsg.classList.add("message_box_success");
    }
  }
}