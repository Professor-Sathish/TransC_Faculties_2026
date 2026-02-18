from flask import Flask, request, jsonify
import subprocess
import os
import struct

app = Flask(__name__)

class CBankInterface:
    def __init__(self, c_program_path="./bank_program"):
        self.c_program_path = c_program_path
        self.data_file = "credit.dat"
    
    def call_c_program(self, choice, input_data=""):
        """Call the C program with proper input handling and encoding"""
        try:
            print(f"Calling C program with choice: {choice}")
            if input_data:
                print(f"Input data: {repr(input_data)}")
            
            # Create the full input string
            if input_data:
                inputs = [choice] + input_data.split('\n') + ['6']
                program_input = '\n'.join(inputs) + '\n'
            else:
                program_input = f"{choice}\n6\n"
            
            print(f"Sending to C program: {repr(program_input)}")
            
            # Use bytes mode to avoid encoding issues
            process = subprocess.Popen(
                [self.c_program_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=False  # Changed to False to handle bytes
            )
            
            # Encode input to bytes
            input_bytes = program_input.encode('utf-8')
            stdout_bytes, stderr_bytes = process.communicate(input=input_bytes, timeout=10)
            
            # Decode output with error handling
            try:
                stdout = stdout_bytes.decode('utf-8')
            except UnicodeDecodeError:
                # Try with latin-1 encoding if utf-8 fails
                stdout = stdout_bytes.decode('latin-1', errors='ignore')
                print("Warning: Had to use latin-1 encoding for stdout")
            
            try:
                stderr = stderr_bytes.decode('utf-8')
            except UnicodeDecodeError:
                stderr = stderr_bytes.decode('latin-1', errors='ignore')
                print("Warning: Had to use latin-1 encoding for stderr")
            
            print(f"C program return code: {process.returncode}")
            print(f"C program output: {stdout}")
            if stderr:
                print(f"C program stderr: {stderr}")
            
            success = process.returncode == 0 and "File could not be opened" not in stdout
            
            return {
                "success": success,
                "output": stdout,
                "error": stderr
            }
                
        except subprocess.TimeoutExpired:
            return {"success": False, "output": "", "error": "Program timed out"}
        except Exception as e:
            print(f"Error calling C program: {e}")
            return {"success": False, "output": "", "error": str(e)}
    
    def read_accounts_from_file(self):
        """Read accounts directly from binary file"""
        accounts = []
        try:
            if os.path.exists(self.data_file):
                with open(self.data_file, "rb") as f:
                    for i in range(100):
                        data = f.read(40)  # 40 bytes per record
                        if len(data) == 40:
                            try:
                                acct_num, last_name_bytes, first_name_bytes, balance = struct.unpack("I15s10sd", data)
                                
                                if acct_num != 0:
                                    last_name = last_name_bytes.decode('utf-8', errors='ignore').rstrip('\x00').strip()
                                    first_name = first_name_bytes.decode('utf-8', errors='ignore').rstrip('\x00').strip()
                                    
                                    if last_name and first_name:
                                        accounts.append({
                                            'acct_num': acct_num,
                                            'last_name': last_name,
                                            'first_name': first_name,
                                            'balance': balance
                                        })
                                        
                            except (struct.error, UnicodeDecodeError):
                                continue
                        else:
                            break
                    
                    print(f"Total accounts found: {len(accounts)}")
                    
        except Exception as e:
            print(f"Error reading accounts: {e}")
        
        return accounts
    
    def add_account(self, account_num, last_name, first_name, balance):
        """Add a new account via C program"""
        input_data = f"{account_num}\n{last_name} {first_name} {balance}"
        return self.call_c_program("3", input_data)
    
    def update_account(self, account_num, transaction):
        """Update account balance via C program"""
        input_data = f"{account_num}\n{transaction}"
        return self.call_c_program("2", input_data)
    
    def delete_account(self, account_num):
        """Delete account via C program"""
        input_data = f"{account_num}"
        return self.call_c_program("4", input_data)
    
    def export_to_text(self):
        """Export accounts to text file via C program"""
        return self.call_c_program("1")
    
    def sort_accounts(self):
        """Sort accounts via C program"""
        return self.call_c_program("5")

# Initialize the interface
bank = CBankInterface()

@app.route('/')
def index():
    return '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Bank Account Manager</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; justify-content: center; align-items: center; padding: 20px; }
        .container { background: rgba(255, 255, 255, 0.95); backdrop-filter: blur(10px); border-radius: 20px; box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1); padding: 40px; max-width: 1000px; width: 100%; }
        h1 { text-align: center; color: #333; margin-bottom: 30px; font-size: 2.5rem; background: linear-gradient(45deg, #667eea, #764ba2); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
        .menu { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; margin-bottom: 30px; }
        .menu-item { background: linear-gradient(45deg, #667eea, #764ba2); color: white; border: none; padding: 20px; border-radius: 15px; font-size: 1.1rem; cursor: pointer; transition: all 0.3s ease; box-shadow: 0 8px 16px rgba(102, 126, 234, 0.3); }
        .menu-item:hover { transform: translateY(-5px); box-shadow: 0 12px 24px rgba(102, 126, 234, 0.4); }
        .form-section { display: none; background: #f8f9fa; padding: 25px; border-radius: 15px; margin-top: 20px; border-left: 5px solid #667eea; }
        .form-section.active { display: block; animation: slideIn 0.3s ease; }
        @keyframes slideIn { from { opacity: 0; transform: translateY(20px); } to { opacity: 1; transform: translateY(0); } }
        .form-group { margin-bottom: 20px; }
        label { display: block; margin-bottom: 8px; font-weight: 600; color: #333; }
        input[type="text"], input[type="number"] { width: 100%; padding: 12px; border: 2px solid #e9ecef; border-radius: 8px; font-size: 1rem; transition: border-color 0.3s ease; }
        input[type="text"]:focus, input[type="number"]:focus { outline: none; border-color: #667eea; box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1); }
        .btn { background: linear-gradient(45deg, #667eea, #764ba2); color: white; border: none; padding: 12px 25px; border-radius: 8px; font-size: 1rem; cursor: pointer; transition: all 0.3s ease; margin-right: 10px; }
        .btn:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(102, 126, 234, 0.3); }
        .btn-secondary { background: #6c757d; }
        .accounts-display { background: #f8f9fa; padding: 20px; border-radius: 15px; margin-top: 20px; max-height: 400px; overflow-y: auto; }
        .account-item { background: white; padding: 15px; margin-bottom: 10px; border-radius: 10px; box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1); display: grid; grid-template-columns: 1fr 2fr 2fr 1fr; gap: 15px; align-items: center; }
        .account-header { font-weight: bold; background: #667eea; color: white; margin-bottom: 15px; }
        .status { padding: 10px; border-radius: 8px; margin-top: 15px; display: none; }
        .status.success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .status.error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .loading { text-align: center; padding: 20px; }
        .spinner { border: 4px solid #f3f3f3; border-top: 4px solid #667eea; border-radius: 50%; width: 40px; height: 40px; animation: spin 1s linear infinite; margin: 0 auto 10px; }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
    </style>
</head>
<body>
    <div class="container">
        <h1>🏦 AB'S Bank Account Manager</h1>
        
        <div class="menu">
            <button class="menu-item" onclick="viewAllAccounts()">📋 View All Accounts</button>
            <button class="menu-item" onclick="showSection('add')">➕ Add New Account</button>
            <button class="menu-item" onclick="showSection('update')">✏️ Update Account</button>
            <button class="menu-item" onclick="showSection('delete')">🗑️ Delete Account</button>
            <button class="menu-item" onclick="exportAccounts()">📄 Export to Text</button>
            <button class="menu-item" onclick="sortAccounts()">🔄 Sort by Balance</button>
        </div>

        <div id="add-section" class="form-section">
            <h3>Add New Account</h3>
            <form onsubmit="addAccount(event)">
                <div class="form-group">
                    <label>Account Number (1-100):</label>
                    <input type="number" id="add-account-num" min="1" max="100" required>
                </div>
                <div class="form-group">
                    <label>Last Name:</label>
                    <input type="text" id="add-last-name" maxlength="14" required>
                </div>
                <div class="form-group">
                    <label>First Name:</label>
                    <input type="text" id="add-first-name" maxlength="9" required>
                </div>
                <div class="form-group">
                    <label>Initial Balance:</label>
                    <input type="number" id="add-balance" step="0.01" required>
                </div>
                <button type="submit" class="btn">Add Account</button>
                <button type="button" class="btn btn-secondary" onclick="clearForm('add')">Clear</button>
            </form>
        </div>

        <div id="update-section" class="form-section">
            <h3>Update Account Balance</h3>
            <form onsubmit="updateAccount(event)">
                <div class="form-group">
                    <label>Account Number:</label>
                    <input type="number" id="update-account-num" min="1" max="100" required>
                </div>
                <div class="form-group">
                    <label>Transaction Amount (+ for charge, - for payment):</label>
                    <input type="number" id="update-transaction" step="0.01" required>
                </div>
                <button type="submit" class="btn">Update Account</button>
                <button type="button" class="btn btn-secondary" onclick="clearForm('update')">Clear</button>
            </form>
        </div>

        <div id="delete-section" class="form-section">
            <h3>Delete Account</h3>
            <form onsubmit="deleteAccount(event)">
                <div class="form-group">
                    <label>Account Number to Delete:</label>
                    <input type="number" id="delete-account-num" min="1" max="100" required>
                </div>
                <button type="submit" class="btn">Delete Account</button>
                <button type="button" class="btn btn-secondary" onclick="clearForm('delete')">Clear</button>
            </form>
        </div>

        <div id="results-section" class="form-section active">
            <h3 id="results-title">Welcome to Bank Account Manager</h3>
            <div id="results-display" class="accounts-display">
                <div style="text-align: center; padding: 40px; color: #666;">
                    👆 Click any button above to see results here
                </div>
            </div>
        </div>

        <div id="status" class="status"></div>
        <div class="footer-credits" style="text-align: center; margin-top: 30px; color: #666;">
        <h4>Developed by <a href="https://www.ashokbakthavathsalam.com/" target="_blank" style="color: #667eea; text-decoration: none;">Ashok Bakthavatchalam</a> & UI by <a href="https://professor-sathish.github.io/" target="_blank" style="color: #667eea; text-decoration: none;">Professor Sathish</a></h4>
        </div>
    </div>
    

    <script>
        function showResults(title, content) {
            document.getElementById('results-title').textContent = title;
            document.getElementById('results-display').innerHTML = content;
            document.querySelectorAll('.form-section').forEach(s => s.classList.remove('active'));
            document.getElementById('results-section').classList.add('active');
        }

        function showSection(section) {
            document.querySelectorAll('.form-section').forEach(s => s.classList.remove('active'));
            document.getElementById(section + '-section').classList.add('active');
        }

        async function apiCall(endpoint, method = 'GET', data = null) {
            try {
                const options = { method, headers: { 'Content-Type' : 'application/json' } };
                if (data) options.body = JSON.stringify(data);
                const response = await fetch(endpoint, options);
                return await response.json();
            } catch (error) {
                return { success: false, message: 'Network error: ' + error.message };
            }
        }

        function showLoading(show) {
            const resultsDisplay = document.getElementById('results-display');
            if (show) {
                resultsDisplay.innerHTML = '<div class="loading"><div class="spinner"></div><p>Processing...</p></div>';
            }
        }

        async function viewAllAccounts() {
            showLoading(true);
            const result = await apiCall('/api/accounts');
            
            if (result.success && result.accounts) {
                let content = '<div class="account-item account-header"><div>Account #</div><div>Last Name</div><div>First Name</div><div>Balance</div></div>';
                
                if (result.accounts.length === 0) {
                    content += '<div style="text-align: center; padding: 20px; color: #666;">No accounts found</div>';
                } else {
                    result.accounts.forEach(account => {
                        content += `<div class="account-item"><div>${account.acct_num}</div><div>${account.last_name}</div><div>${account.first_name}</div><div>$${account.balance.toFixed(2)}</div></div>`;
                    });
                }
                
                showResults(`📋 All Accounts (${result.accounts.length} found)`, content);
            } else {
                showResults('❌ Error', '<div style="text-align: center; padding: 20px; color: #dc3545;">Failed to load accounts</div>');
            }
        }

        async function sortAccounts() {
            showLoading(true);
            const result = await apiCall('/api/accounts/sort', 'POST');
            
            if (result.success && result.accounts) {
                let content = '<div class="account-item account-header"><div>Rank</div><div>Account #</div><div>Last Name</div><div>First Name</div><div>Balance</div></div>';
                
                result.accounts.forEach((account, index) => {
                    content += `<div class="account-item" style="background: linear-gradient(45deg, #f8f9fa, #e9ecef);"><div style="color: #667eea; font-weight: bold;">#${index + 1}</div><div>${account.acct_num}</div><div>${account.last_name}</div><div>${account.first_name}</div><div style="font-weight: bold; color: #28a745;">$${account.balance.toFixed(2)}</div></div>`;
                });
                
                showResults(`🏆 Accounts Sorted by Balance (${result.accounts.length} accounts)`, content);
                showStatus('✅ Accounts sorted successfully!', 'success');
            } else {
                showResults('❌ Sort Failed', '<div style="text-align: center; padding: 20px; color: #dc3545;">Failed to sort accounts</div>');
            }
        }

        async function exportAccounts() {
            showLoading(true);
            const result = await apiCall('/api/accounts/export', 'POST');
            
            const content = `<div style="text-align: center; padding: 40px;"><div style="font-size: 48px; margin-bottom: 20px;">${result.success ? '✅' : '❌'}</div><h3>${result.success ? 'Export Successful!' : 'Export Failed'}</h3><p style="margin: 20px 0; color: #666;">${result.message}</p>${result.success ? '<p style="color: #28a745;"><strong>File saved as: accounts.txt</strong></p>' : ''}</div>`;
            
            showResults('📄 Export Result', content);
            showStatus(result.message, result.success ? 'success' : 'error');
        }

        async function addAccount(event) {
            event.preventDefault();
            const data = {
                account_num: document.getElementById('add-account-num').value,
                last_name: document.getElementById('add-last-name').value,
                first_name: document.getElementById('add-first-name').value,
                balance: document.getElementById('add-balance').value
            };
            
            showLoading(true);
            const result = await apiCall('/api/accounts/add', 'POST', data);
            
            const content = `<div style="text-align: center; padding: 40px;"><div style="font-size: 48px; margin-bottom: 20px;">${result.success ? '✅' : '❌'}</div><h3>${result.success ? 'Account Added Successfully!' : 'Failed to Add Account'}</h3><p style="margin: 20px 0; color: #666;">${result.message}</p>${result.success ? `<div style="background: #f8f9fa; padding: 15px; border-radius: 8px; margin: 20px 0;"><strong>Account Details:</strong><br>Account #: ${data.account_num}<br>Name: ${data.first_name} ${data.last_name}<br>Balance: $${parseFloat(data.balance).toFixed(2)}</div>` : ''}</div>`;
            
            showResults('➕ Add Account Result', content);
            showStatus(result.message, result.success ? 'success' : 'error');
            
            if (result.success) clearForm('add');
        }

        async function updateAccount(event) {
            event.preventDefault();
            const data = {
                account_num: document.getElementById('update-account-num').value,
                transaction: document.getElementById('update-transaction').value
            };
            
            showLoading(true);
            const result = await apiCall('/api/accounts/update', 'POST', data);
            
            const content = `<div style="text-align: center; padding: 40px;"><div style="font-size: 48px; margin-bottom: 20px;">${result.success ? '✅' : '❌'}</div><h3>${result.success ? 'Account Updated Successfully!' : 'Failed to Update Account'}</h3><p style="margin: 20px 0; color: #666;">${result.message}</p>${result.success ? `<div style="background: #f8f9fa; padding: 15px; border-radius: 8px; margin: 20px 0;"><strong>Transaction Details:</strong><br>Account #: ${data.account_num}<br>Transaction: ${parseFloat(data.transaction) >= 0 ? '+' : ''}$${parseFloat(data.transaction).toFixed(2)}</div>` : ''}</div>`;
            
            showResults('✏️ Update Account Result', content);
            showStatus(result.message, result.success ? 'success' : 'error');
            
            if (result.success) clearForm('update');
        }

        async function deleteAccount(event) {
            event.preventDefault();
            if (!confirm('Are you sure you want to delete this account?')) return;
            
            const data = { account_num: document.getElementById('delete-account-num').value };
            
            showLoading(true);
            const result = await apiCall('/api/accounts/delete', 'POST', data);
            
            const content = `<div style="text-align: center; padding: 40px;"><div style="font-size: 48px; margin-bottom: 20px;">${result.success ? '✅' : '❌'}</div><h3>${result.success ? 'Account Deleted Successfully!' : 'Failed to Delete Account'}</h3><p style="margin: 20px 0; color: #666;">${result.message}</p>${result.success ? `<div style="background: #f8f9fa; padding: 15px; border-radius: 8px; margin: 20px 0;"><strong>Deleted Account #: ${data.account_num}</strong></div>` : ''}</div>`;
            
            showResults('🗑️ Delete Account Result', content);
            showStatus(result.message, result.success ? 'success' : 'error');
            
            if (result.success) clearForm('delete');
        }

        function clearForm(section) {
            document.querySelectorAll(`#${section}-section input`).forEach(input => input.value = '');
        }

        function showStatus(message, type) {
            const statusElement = document.getElementById('status');
            statusElement.textContent = message;
            statusElement.className = `status ${type}`;
            statusElement.style.display = 'block';
            setTimeout(() => statusElement.style.display = 'none', 3000);
        }
       
        </script>
</body>
</html>'''

@app.route('/api/accounts', methods=['GET'])
def get_accounts():
    """Get all accounts"""
    try:
        accounts = bank.read_accounts_from_file()
        
        # Fallback to text file if binary reading fails
        if not accounts and os.path.exists("accounts.txt"):
            print("No accounts from binary file, trying accounts.txt...")
            try:
                with open("accounts.txt", "r") as f:
                    lines = f.readlines()
                    for line in lines[1:]:  # Skip header
                        if line.strip() and not line.startswith('-'):
                            parts = line.strip().split()
                            if len(parts) >= 4:
                                try:
                                    acct_num = int(parts[0])
                                    last_name = parts[1]
                                    first_name = parts[2]
                                    balance = float(parts[3])
                                    accounts.append({
                                        'acct_num': acct_num,
                                        'last_name': last_name,
                                        'first_name': first_name,
                                        'balance': balance
                                    })
                                except (ValueError, IndexError):
                                    continue
                print(f"Found {len(accounts)} accounts from text file")
            except Exception as e:
                print(f"Error reading from accounts.txt: {e}")
        
        return jsonify({"success": True, "accounts": accounts})
    except Exception as e:
        return jsonify({"success": False, "message": str(e), "accounts": []})

@app.route('/api/accounts/add', methods=['POST'])
def add_account():
    """Add a new account"""
    data = request.get_json()
    
    try:
        account_num = int(data['account_num'])
        last_name = data['last_name'].strip()
        first_name = data['first_name'].strip()
        balance = float(data['balance'])
        
        if account_num < 1 or account_num > 100:
            return jsonify({"success": False, "message": "Account number must be between 1 and 100"})
        
        if not last_name or not first_name:
            return jsonify({"success": False, "message": "First and last name are required"})
        
        result = bank.add_account(account_num, last_name, first_name, balance)
        
        if result['success']:
            return jsonify({"success": True, "message": "Account added successfully!"})
        else:
            if "already contains information" in result['output']:
                return jsonify({"success": False, "message": f"Account #{account_num} already exists"})
            else:
                return jsonify({"success": False, "message": "Failed to add account"})
            
    except ValueError:
        return jsonify({"success": False, "message": "Invalid input values"})
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})

@app.route('/api/accounts/update', methods=['POST'])
def update_account():
    """Update account balance"""
    data = request.get_json()
    
    try:
        account_num = int(data['account_num'])
        transaction = float(data['transaction'])
        
        result = bank.update_account(account_num, transaction)
        
        if result['success']:
            return jsonify({"success": True, "message": "Account updated successfully!"})
        else:
            if "has no information" in result['output']:
                return jsonify({"success": False, "message": f"Account #{account_num} does not exist"})
            else:
                return jsonify({"success": False, "message": "Failed to update account"})
            
    except ValueError:
        return jsonify({"success": False, "message": "Invalid input values"})
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})

@app.route('/api/accounts/delete', methods=['POST'])
def delete_account():
    """Delete an account"""
    data = request.get_json()
    
    try:
        account_num = int(data['account_num'])
        
        result = bank.delete_account(account_num)
        
        if result['success']:
            return jsonify({"success": True, "message": "Account deleted successfully!"})
        else:
            if "does not exist" in result['output']:
                return jsonify({"success": False, "message": f"Account #{account_num} does not exist"})
            else:
                return jsonify({"success": False, "message": "Failed to delete account"})
            
    except ValueError:
        return jsonify({"success": False, "message": "Invalid account number"})
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})

@app.route('/api/accounts/export', methods=['POST'])
def export_accounts():
    """Export accounts to text file"""
    try:
        result = bank.export_to_text()
        
        if result['success']:
            return jsonify({"success": True, "message": "Accounts exported to accounts.txt"})
        else:
            return jsonify({"success": False, "message": "Failed to export accounts"})
            
    except Exception as e:
        return jsonify({"success": False, "message": str(e)})

@app.route('/api/accounts/sort', methods=['POST'])
def sort_accounts():
    """Sort accounts by balance using C program"""
    try:
        print("=== 🔄 STARTING SORT OPERATION ===")
        
        result = bank.sort_accounts()
        
        print(f"C program success: {result['success']}")
        print(f"Raw output: {repr(result['output'])}")
        
        if result['success']:
            lines = result['output'].split('\n')
            sorted_accounts = []
            
            print(f"Total lines received: {len(lines)}")
            
            for i, line in enumerate(lines):
                line = line.strip()
                print(f"Line {i}: '{line}'")
                
                if not line:
                    continue
                    
                # Skip header line and menu prompts
                if ("Acct" in line and "Last Name" in line) or "Enter your choice" in line or "?" in line:
                    continue
                
                parts = line.split()
                print(f"  Parts: {parts}")
                
                if len(parts) >= 4:
                    try:
                        acct_num = int(parts[0])
                        last_name = parts[1]
                        first_name = parts[2]
                        balance = float(parts[3])
                        
                        # Skip if it looks like a menu option
                        if acct_num > 100:
                            continue
                        
                        sorted_accounts.append({
                            'acct_num': acct_num,
                            'last_name': last_name,
                            'first_name': first_name,
                            'balance': balance
                        })
                        
                        print(f"✅ Parsed: #{acct_num} {last_name} {first_name} ${balance:,.2f}")
                        
                    except (ValueError, IndexError) as e:
                        print(f"  Parse error: {e}")
                        continue
            
            print(f"=== 📊 PARSED {len(sorted_accounts)} SORTED ACCOUNTS ===")
            
            if sorted_accounts:
                print("🏆 FINAL SORTED ORDER:")
                for i, acc in enumerate(sorted_accounts, 1):
                    print(f"  {i}. #{acc['acct_num']} {acc['last_name']} {acc['first_name']} - ${acc['balance']:,.2f}")
                
                return jsonify({
                    "success": True, 
                    "message": f"🎯 Sorted by balance! Top: {sorted_accounts[0]['last_name']} ${sorted_accounts[0]['balance']:,.2f}",
                    "accounts": sorted_accounts
                })
            else:
                print("❌ No accounts parsed from C program output")
                print("Full output was:", repr(result['output']))
                
                # Fallback: try to read accounts and sort in Python
                accounts = bank.read_accounts_from_file()
                if accounts:
                    sorted_accounts = sorted(accounts, key=lambda x: x['balance'], reverse=True)
                    print(f"📋 Using Python sort fallback: {len(sorted_accounts)} accounts")
                    return jsonify({
                        "success": True, 
                        "message": "Sorted by balance (Python fallback)",
                        "accounts": sorted_accounts
                    })
                else:
                    return jsonify({"success": False, "message": "No accounts to sort"})
        else:
            print("❌ C program failed")
            print(f"Error: {result['error']}")
            return jsonify({"success": False, "message": f"C program failed: {result['error']}"})
            
    except Exception as e:
        print(f"❌ SORT ERROR: {e}")
        return jsonify({"success": False, "message": str(e)})

@app.route('/api/test', methods=['GET'])
def test_c_program():
    """Test endpoint to verify C program communication"""
    result = bank.call_c_program("6")  # Just test with exit command
    return jsonify({
        "c_program_working": result['success'],
        "output": result['output'],
        "error": result['error']
    })

if __name__ == '__main__':
    print("=== Bank Account Manager with C Backend ===")
    print("Your C program is working correctly!")
    print("Test the connection: http://localhost:5000/api/test")
    print("Main interface: http://localhost:5000")
    print("===========================================")
    app.run(debug=True, host='0.0.0.0', port=5000)