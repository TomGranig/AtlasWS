let previousUsername = null; // Keep track of the previous username to avoid repeating it in the chat log


class WebSocketClient {
    constructor(url) {
        this.url = url;
        this.socket = new WebSocket(url);
        this.bindEvents();

        setInterval(() => {
            this.send("keepalive");
        }, 5000);
    }

    bindEvents() {
        this.socket.onopen = this.onOpen.bind(this);
        this.socket.onmessage = this.onMessage.bind(this);
        this.socket.onclose = this.onClose.bind(this);
        this.socket.onerror = this.onError.bind(this);
    }

    onOpen(event) {
        console.log("WebSocket connection opened:", event);
    }

    onMessage(event) {
        console.log("WebSocket message received:", event);
        const message = event.data;
        appendMessages([message]);
    }

    onClose(event) {
        console.log("WebSocket connection closed:", event);

        setTimeout(() => {
            // clear the chat log
            document.getElementById("chat-log").innerHTML = "";

            this.socket = new WebSocket(this.url);
            this.bindEvents();
        }, 1000);
    }

    onError(event) {
        console.error("WebSocket error:", event);
    }

    send(message) {
        if (this.socket.readyState !== WebSocket.OPEN) {
            return;
        }

        this.socket.send(message);
    }
}

const wsClient = new WebSocketClient(window.location.origin.replace("http", "ws").replace("https", "wss") + "/ws");


function sendMessage() {
    const username = document.getElementById("username").value; // Get the username from the input field
    const message = document.getElementById("message").value; // Get the message from the input field

    if (!username || !message) { // If the username or message is empty, do nothing
        return;
    }

    // fetch("/send?username=" + username, { // Send a POST request to the server with the username and message in the body
    //     method: "POST",
    //     body: message
    // });

    const messageLine = username + ":" + message;
    wsClient.send(messageLine);

    appendMessages([messageLine]); // Append the message line to the chat log

    document.getElementById("message").value = ""; // Clear the message input field
    document.getElementById("message").focus(); // Focus the message input field
}

function appendMessages(messages) {
    let currentUserUsername = document.getElementById("username").value; // Get the current user's username from the input field

    for (const message of messages) { // Loop through each message in the array
        const colonIndex = message.indexOf(":"); // Find the index of the colon character in the message
        if (colonIndex < 0) { // If there is no colon character, skip this message
            continue;
        }

        const username = message.slice(0, colonIndex); // Extract the username from the message
        const text = message.slice(colonIndex + 1); // Extract the text from the message

        const messageContainer = document.createElement("div"); // Create a new div element to hold the message
        messageContainer.classList.add("message-container"); // Add a CSS class to the message container

        const usernameElement = document.createElement("div"); // Create a new div element to hold the username
        usernameElement.classList.add("username"); // Add a CSS class to the username element
        usernameElement.innerText = username; // Set the text of the username element to the username

        const messageContentElement = document.createElement("div"); // Create a new div element to hold the message content
        messageContentElement.classList.add("content"); // Add a CSS class to the message content element
        messageContentElement.innerText = text; // Set the text of the message content element to the message text

        if (username === currentUserUsername) { // If the message is from the current user, add a CSS class to the message container
            messageContainer.classList.add("current-user");
        }

        if (username !== previousUsername) { // If the message is from a different user than the previous message, add the username element to the message container
            messageContainer.appendChild(usernameElement);
        }

        previousUsername = username; // Set the previous username to the current username
        messageContainer.appendChild(messageContentElement); // Add the message content element to the message container
        document.getElementById("chat-log").appendChild(messageContainer); // Add the message container to the chat log
    }

    scrollToBottom(); // Scroll to the bottom of the chat log
}

// async function getMessages() {
//     const response = await fetch("/get-messages"); // Send a GET request to the server to get the messages
//     const text = await response.text(); // Get the response text
//     const messages = text.split("\n"); // Split the response text into an array of messages
//     appendMessages(messages); // Append the messages to the chat log
// }

// async function waitForMessages() {
//     // this is not ideal in reality because some messages may not be received if the timeout is reached and the client 
//     // has to call waitForMessages() again and during this time the server may have sent more messages that the client
//     // will not receive until the next request is received by the server
//     try {
//         const response = await fetch("/wait-for-messages"); // Send a GET request to the server to wait for new messages, the request will not return until there are new messages or the timeout is reached
//         const text = await response.text(); // Get the response text
//         const messages = text.split("\n"); // Split the response text into an array of messages
//         appendMessages(messages); // Append the new messages to the chat log
//         waitForMessages(); // Call this function again to wait for more messages
//     } catch (e) {
//         waitForMessages(); // Call this function again to wait for more messages
//     }
// }

function generateUsername() {
    const newUsername = "user" + Math.floor(Math.random() * 100000); // Generate a random username
    localStorage.setItem("username", newUsername); // Save the username to local storage
    return newUsername; // Return the username
}

function scrollToBottom() {
    const chatLog = document.getElementById("chat-log");

    if (window.SCROLL_TO_BOTTOM_IMMEDIATE) { // If the SCROLL_TO_BOTTOM_IMMEDIATE flag is set, scroll to the bottom immediately
        chatLog.scrollTop = chatLog.scrollHeight;
        return;
    }

    let cancelScrollAnimation = false; // Create a flag to cancel the scroll animation if the user scrolls manually

    chatLog.onwheel = () => { // Add an event listener for the wheel event (scrolling with a mouse or trackpad)
        cancelScrollAnimation = true; // Set the cancel flag to true
    };

    chatLog.ontouchstart = () => { // Add an event listener for the touchstart event (start touching the screen)
        cancelScrollAnimation = true; // Set the cancel flag to true
    };

    const scrollDown = () => { // Define a function to scroll down the chat log smoothly (ease out)
        const dy = (chatLog.scrollHeight - chatLog.scrollTop - chatLog.clientHeight) / 15; // Calculate the distance to scroll based on the current scroll position and the height of the chat log
        chatLog.scrollTop += dy; // Scroll down by the calculated distance
        if (dy < 1.01 || cancelScrollAnimation) { // If the distance is less than 1 or the cancel flag is set, stop scrolling
            return;
        }
        requestAnimationFrame(scrollDown); // Call this function again on the next animation frame
    };

    requestAnimationFrame(scrollDown); // Call the scrollDown function on the next animation frame
}

document.getElementById("send").addEventListener("click", sendMessage); // Add an event listener for the send button

document.getElementById("message").addEventListener("keyup", e => { // Add an event listener for the keyup event (when the user releases a key)
    if (e.key === "Enter") { // If the key is the Enter key, send the message
        sendMessage();
    }
});

document.getElementById("username").onchange = () => { // Add an event listener for the change event (when the user changes the username)
    document.getElementById("chat-log").innerHTML = ""; // Clear the chat log
    getMessages(); // Get the messages from the server
    localStorage.setItem("username", document.getElementById("username").value); // Save the username to local storage
};

document.getElementById("username").value = localStorage.getItem("username") || generateUsername(); // Set the username input field to the saved username or a generated username

// getMessages(); // Get the messages from the server
// waitForMessages(); // Wait for new messages from the server