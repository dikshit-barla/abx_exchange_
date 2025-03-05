# ABX Exchange Client

## Prerequisites
- Node.js (version 16.17.0 or higher)
- C++ Compiler (g++)
  
## Setup Instructions

### 1. Open Command Prompt or Terminal
- **Windows**: Press `Win + R`, type `cmd`, and hit `Enter`.
- **Mac/Linux**: Open the Terminal from the applications menu.
- Navigate to the directory where you want to clone the repository.

### 2. Clone the Repository
```bash
git clone https://github.com/dikshit-barla/abx_exchange_.git
cd abx_exchange_
```

### 3. Navigate to Server Directory
```bash
cd abx_exchange_server
```

### 4. Start the Server
1. Ensure you have Node.js installed  
2. Run the server:  
```bash
node main.js
```
- The server will start on port 3000  
- You should see the message **"TCP server started on port 3000"**  

### 5. Open a New Command Prompt or Terminal Window
- **Windows**: Press `Win + R`, type `cmd`, and hit `Enter`.
- **Mac/Linux**: Open a new terminal session.

### 6. Navigate to Client Directory
```bash
cd abx_exchange_
cd abx_exchange_client
```

### 7. Compile the C++ Client
```bash
g++ -std=c++11 client.cpp -o abx_client
```

### 8. Run the Client
```bash
./abx_client
```

### Expected Output
- The client will connect to the server  
- Retrieve stock ticker data  
- Generate `output.json` file  
- Display processing statistics in the terminal  

## Troubleshooting
- Ensure Node.js is version **16.17.0 or higher**  
- Verify that **port 3000 is available**  
- Check **network connectivity**  
- Confirm **C++ compiler is installed**  

## Notes
- The `output.json` file will be generated in the same directory  
- The JSON contains all received market data packets  

