import gradio as gr
import requests
import pandas as pd
import subprocess
import threading
import time
from your_flask_app import app  # Import your Flask app

# Start Flask app in background
def start_flask():
    app.run(host='0.0.0.0', port=5000, debug=False)

# Start Flask in a separate thread
flask_thread = threading.Thread(target=start_flask, daemon=True)
flask_thread.start()
time.sleep(2)  # Wait for Flask to start

BASE_URL = "http://localhost:5000/api"

def call_api(endpoint, method="GET", data=None):
    """Call Flask API"""
    try:
        if method == "GET":
            response = requests.get(f"{BASE_URL}{endpoint}")
        else:
            response = requests.post(f"{BASE_URL}{endpoint}", json=data)
        return response.json()
    except Exception as e:
        return {"success": False, "message": str(e)}

def view_all_accounts():
    result = call_api("/accounts")
    if result.get("success") and result.get("accounts"):
        df = pd.DataFrame([
            {
                'Account #': acc['acct_num'],
                'Last Name': acc['last_name'],
                'First Name': acc['first_name'],
                'Balance': f"${acc['balance']:.2f}"
            }
            for acc in result["accounts"]
        ])
        return df, f"Found {len(result['accounts'])} accounts"
    return pd.DataFrame(), "No accounts found"

# ... rest of the functions using API calls

# Create Gradio interface (same as Option 1)
