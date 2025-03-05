# ABX Exchange Client - Setup & Usage Guide

## Prerequisites

- **Node.js** (Version **16.17.0** or higher)
- **C++ Compiler** (e.g., **g++**)

## Setup Instructions

### 1. Open Command Prompt or Terminal

- **Windows:** Press **Win + R**, type `cmd`, and hit **Enter**.
- **Mac/Linux:** Open the **Terminal** from the applications menu.

### 2. Clone the Repository

Navigate to the directory where you want to clone the repository, then run:

```bash
git clone https://github.com/dikshit-barla/abx_exchange_.git
cd abx_exchange_
```

### 3. Navigate to the Server Directory

```bash
cd abx_exchange_server
```

### 4. Start the Server

Ensure **Node.js** is installed, then run:

```bash
node main.js
```

You should see:

```
TCP server started on port 3000
```

### 5. Open a New Command Prompt or Terminal Window

- **Windows:** Press **Win + R**, type `cmd`, and hit **Enter**.
- **Mac/Linux:** Open a new terminal session.

### 6. Navigate to the Client Directory

```bash
cd abx_exchange_
cd abx_exchange_client
```

### 7. Compile the C++ Client

Use the following command to compile the client:

```bash
g++ -std=c++11 client.cpp -o abx_client -lws2_32
```

### 8. Run the Client

Execute the compiled client:

```bash
./abx_client
```

---

## Expected Output

- The client will **connect** to the server.
- It will **retrieve stock ticker data**.
- A **`output.json`** file will be generated in the same directory.
- The terminal will display **processing statistics**.

---

## Troubleshooting

If you face any issues, check the following:

- Ensure **Node.js** is **version 16.17.0 or higher**.
- Verify that **port 3000** is available and not blocked.
- Check your **network connectivity**.
- Confirm that your **C++ compiler** is installed correctly.

---

## Important Note

- The generated **`output.json`** file contains all received market data packets.
- Open the `output.json` file to inspect the retrieved stock market data.

