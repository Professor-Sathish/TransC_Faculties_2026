from flask import Flask, request, jsonify
import subprocess
import os
import struct

app = Flask(__name__)

class CBankInterface:
    def __init__(self, c_program_path="./i7"):
        
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

            return {"success": success, "output": stdout, "error": stderr}

        except subprocess.TimeoutExpired:
            return {"success": False, "output": "", "error": "Program timed out"}
        except Exception as e:
            print(f"Error calling C program: {e}")
            return {"success": False, "output": "", "error": str(e)}

    def read_accounts_from_file(self):
        """Read accounts directly from binary file with V6 validation"""
        accounts = []
        try:
            if os.path.exists(self.data_file):
                with open(self.data_file, "rb") as f:
                    for i in range(100):
                        data = f.read(40)  # 40 bytes per record
                        if len(data) == 40:
                            try:
                                acct_num, last_name_bytes, first_name_bytes, balance = struct.unpack("I15s10sd", data)

                                # V6 validation: valid account number and reasonable balance
                                if (acct_num != 0 and 
                                    acct_num >= 1 and 
                                    acct_num <= 100 and
                                    balance >= -1000000.0 and 
                                    balance <= 10000000.0):

                                    last_name = last_name_bytes.decode('utf-8', errors='ignore').rstrip('\x00').strip()
                                    first_name = first_name_bytes.decode('utf-8', errors='ignore').rstrip('\x00').strip()

                                    # Clean non-printable characters (V6 sanitization)
                                    last_name = ''.join(c for c in last_name if c.isprintable())
                                    first_name = ''.join(c for c in first_name if c.isprintable())

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

                print(f"Total valid accounts found: {len(accounts)}")

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

    def sort_accounts(self, criterion, order):
        """Sort accounts via C program with V6 features"""
        input_data = f"{criterion}\n{order}"
        return self.call_c_program("5", input_data)

# Initialize the interface
bank = CBankInterface()

@app.route('/')
def index():
    return '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>V6 Bank Account Manager</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; justify-content: center; align-items: center; padding: 20px; }
        .container { background: rgba(255, 255, 255, 0.95); backdrop-filter: blur(10px); border-radius: 20px; box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1); padding: 40px; max-width: 1200px; width: 100%; }
        h1 { text-align: center; color: #333; margin-bottom: 30px; font-size: 2.5rem; background: linear-gradient(45deg, #667eea, #764ba2); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
        .version-badge { background: linear-gradient(45deg, #28a745, #20c997); color: white; padding: 5px 15px; border-radius: 15px; font-size: 0.8rem; font-weight: bold; display: inline-block; margin-bottom: 20px; }
        .menu { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 20px; margin-bottom: 30px; }
        .menu-item { background: linear-gradient(45deg, #667eea, #764ba2); color: white; border: none; padding: 20px; border-radius: 15px; font-size: 1.1rem; cursor: pointer; transition: all 0.3s ease; box-shadow: 0 8px 16px rgba(102, 126, 234, 0.3); position: relative; overflow: hidden; }
        .menu-item:hover { transform: translateY(-5px); box-shadow: 0 12px 24px rgba(102, 126, 234, 0.4); }
        .menu-item::before { content: ''; position: absolute; top: 0; left: -100%; width: 100%; height: 100%; background: linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent); transition: left 0.5s; }
        .menu-item:hover::before { left: 100%; }
        .form-section { display: none; background: #f8f9fa; padding: 25px; border-radius: 15px; margin-top: 20px; border-left: 5px solid #667eea; }
        .form-section.active { display: block; animation: slideIn 0.3s ease; }
        @keyframes slideIn { from { opacity: 0; transform: translateY(20px); } to { opacity: 1; transform: translateY(0); } }
        .form-group { margin-bottom: 20px; }
        label { display: block; margin-bottom: 8px; font-weight: 600; color: #333; }
        input[type="text"], input[type="number"], select { width: 100%; padding: 12px; border: 2px solid #e9ecef; border-radius: 8px; font-size: 1rem; transition: border-color 0.3s ease; }
        input[type="text"]:focus, input[type="number"]:focus, select:focus { outline: none; border-color: #667eea; box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1); }
        .btn { background: linear-gradient(45deg, #667eea, #764ba2); color: white; border: none; padding: 12px 25px; border-radius: 8px; font-size: 1rem; cursor: pointer; transition: all 0.3s ease; margin-right: 10px; }
        .btn:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(102, 126, 234, 0.3); }
        .btn-secondary { background: #6c757d; }
        .btn-success { background: linear-gradient(45deg, #28a745, #20c997); }
        .btn-warning { background: linear-gradient(45deg, #ffc107, #fd7e14); }
        .btn-info { background: linear-gradient(45deg, #17a2b8, #6f42c1); }
        .btn-danger { background: linear-gradient(45deg, #dc3545, #e83e8c); }
        .accounts-display { background: #f8f9fa; padding: 20px; border-radius: 15px; margin-top: 20px; max-height: 500px; overflow-y: auto; }
        .account-item { background: white; padding: 15px; margin-bottom: 10px; border-radius: 10px; box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1); display: grid; grid-template-columns: 1fr 2fr 2fr 1.5fr; gap: 15px; align-items: center; }
        .account-header { font-weight: bold; background: #667eea; color: white; margin-bottom: 15px; }
        .rank-badge { background: linear-gradient(45deg, #ffd700, #ffb347); color: #333; padding: 5px 10px; border-radius: 15px; font-weight: bold; font-size: 0.9rem; }
        .balance-highlight { font-weight: bold; color: #28a745; font-size: 1.1rem; }
        .status { padding: 10px; border-radius: 8px; margin-top: 15px; display: none; }
        .status.success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .status.error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .loading { text-align: center; padding: 20px; }
        .spinner { border: 4px solid #f3f3f3; border-top: 4px solid #667eea; border-radius: 50%; width: 40px; height: 40px; animation: spin 1s linear infinite; margin: 0 auto 10px; }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
        .sort-options { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin: 20px 0; }
        .sort-button { padding: 15px; background: white; border: 2px solid #e9ecef; border-radius: 10px; cursor: pointer; transition: all 0.3s ease; text-align: center; }
        .sort-button:hover { border-color: #667eea; background: #f8f9fa; transform: translateY(-2px); }
        .sort-button.selected { border-color: #667eea; background: #667eea; color: white; }
        .feature-highlight { background: linear-gradient(45deg, #fff3cd, #ffeaa7); padding: 15px; border-radius: 10px; margin: 15px 0; border-left: 4px solid #ffc107; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🏦 AB'S Bank Account Manager</h1>

        <div class="feature-highlight">
            <strong>🚀 V6 Enhanced Features:</strong> Sort by Balance/Name (Asc/Desc) • Find Max/Min Balance • Data Validation • Corruption Filtering
        </div>

        <div class="menu">
            <button class="menu-item" onclick="viewAllAccounts()">📋 View All Accounts</button>
            <button class="menu-item" onclick="showSection('add')">➕ Add New Account</button>
            <button class="menu-item" onclick="showSection('update')">✏️ Update Account</button>
            <button class="menu-item" onclick="showSection('delete')">🗑️ Delete Account</button>
            <button class="menu-item" onclick="exportAccounts()">📄 Export to Text</button>
            <button class="menu-item" onclick="showSection('sort')">🔄 Advanced Sorting</button>
        </div>

        <div id="add-section" class="form-section">
            <h3>➕ Add New Account</h3>
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
            <h3>✏️ Update Account Balance</h3>
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
            <h3>🗑️ Delete Account</h3>
            <form onsubmit="deleteAccount(event)">
                <div class="form-group">
                    <label>Account Number to Delete:</label>
                    <input type="number" id="delete-account-num" min="1" max="100" required>
                </div>
                <button type="submit" class="btn btn-danger">Delete Account</button>
                <button type="button" class="btn btn-secondary" onclick="clearForm('delete')">Clear</button>
            </form>
        </div>

        <div id="sort-section" class="form-section">
            <h3>🔄 Advanced Sorting Options (V6)</h3>
            <div class="sort-options">
                <div class="sort-button" onclick="sortAccounts(1, 1)">
                    <strong>💰 Balance ↑</strong><br>
                    <small>Low to High</small>
                </div>
                <div class="sort-button" onclick="sortAccounts(1, 2)">
                    <strong>💰 Balance ↓</strong><br>
                    <small>High to Low</small>
                </div>
                <div class="sort-button" onclick="sortAccounts(2, 1)">
                    <strong>📝 Name A-Z</strong><br>
                    <small>Alphabetical</small>
                </div>
                <div class="sort-button" onclick="sortAccounts(2, 2)">
                    <strong>📝 Name Z-A</strong><br>
                    <small>Reverse Alpha</small>
                </div>
                <div class="sort-button" onclick="sortAccounts(3, 1)">
                    <strong>🏆 Max Balance</strong><br>
                    <small>Highest Account</small>
                </div>
                <div class="sort-button" onclick="sortAccounts(4, 1)">
                    <strong>📉 Min Balance</strong><br>
                    <small>Lowest Account</small>
                </div>
            </div>
        </div>

        <div id="results-section" class="form-section active">
            <h3 id="results-title">Welcome to AB'S Bank Account Manager</h3>
            <div id="results-display" class="accounts-display">
                <div style="text-align: center; padding: 40px; color: #666;">
                    <div style="font-size: 48px; margin-bottom: 20px;">🏦</div>
                    <h3>AB'S Advanced Banking System</h3>
                    <p>👆 Click any button above to see results here</p>
                    <div style="margin-top: 20px; font-size: 0.9rem; color: #999;">
                        V6 Features: Enhanced Sorting • Data Validation • Corruption Filtering
                    </div>
                </div>
            </div>
        </div>

        <div id="status" class="status"></div>

        <div class="footer-credits" style="text-align: center; margin-top: 30px; color: #666;">
            <h4>Developed by <a href="https://www.ashokbakthavathsalam.com/" target="_blank" style="color: #667eea; text-decoration: none;">Ashok Bakthavatchalam</a> & UI by <a href="https://professor-sathish.github.io/" target="_blank" style="color: #667eea; text-decoration: none;">Professor Sathish</a></h4>
            <p style="font-size: 0.9rem; margin-top: 10px;">🚀 V6 Enhanced with Advanced Sorting, Data Validation & Corruption Filtering</p>
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
                const options = { method, headers: { 'Content-Type': 'application/json' } };
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
                resultsDisplay.innerHTML = '<div class="loading"><div class="spinner"></div><p>Processing with V6 C Program...</p></div>';
            }
        }

        async function viewAllAccounts() {
            showLoading(true);
            const result = await apiCall('/api/accounts');

            if (result.success && result.accounts) {
                let content = '<div class="account-item account-header"><div>Account #</div><div>Last Name</div><div>First Name</div><div>Balance</div></div>';

                if (result.accounts.length === 0) {
                    content += '<div style="text-align: center; padding: 20px; color: #666;">No valid accounts found</div>';
                } else {
                    result.accounts.forEach(account => {
                        content += `<div class="account-item">
                            <div>${account.acct_num}</div>
                            <div>${account.last_name}</div>
                            <div>${account.first_name}</div>
                            <div class="balance-highlight">₹${account.balance.toFixed(2)}</div>
                        </div>`;
                    });
                }

                showResults(`📋 All Accounts (${result.accounts.length} found)`, content);
            } else {
                showResults('❌ Error', '<div style="text-align: center; padding: 20px; color: #dc3545;">Failed to load accounts</div>');
            }
        }

        async function sortAccounts(criterion, order) {
            showLoading(true);

            const sortNames = {
                1: { 1: 'Balance (Low to High)', 2: 'Balance (High to Low)' },
                2: { 1: 'Name (A-Z)', 2: 'Name (Z-A)' },
                3: { 1: 'Maximum Balance Account' },
                4: { 1: 'Minimum Balance Account' }
            };

            const result = await apiCall('/api/accounts/sort', 'POST', { criterion, order });

            if (result.success) {
                let content = '';
                const sortTitle = sortNames[criterion][order];

                if (criterion === 3 || criterion === 4) {
                    // Max/Min display
                    content = '<div class="account-item account-header"><div>Account #</div><div>Last Name</div><div>First Name</div><div>Balance</div></div>';
                    if (result.account) {
                        content += `<div class="account-item" style="background: linear-gradient(45deg, #fff3cd, #ffeaa7);">
                            <div style="font-weight: bold;">${result.account.acct_num}</div>
                            <div>${result.account.last_name}</div>
                            <div>${result.account.first_name}</div>
                            <div class="balance-highlight" style="font-size: 1.2rem;">₹${result.account.balance.toFixed(2)}</div>
                        </div>`;
                    }
                } else {
                    // Sorted list display
                    content = '<div class="account-item account-header"><div>Rank</div><div>Account #</div><div>Last Name</div><div>First Name</div><div>Balance</div></div>';
                    if (result.accounts && result.accounts.length > 0) {
                        result.accounts.forEach((account, index) => {
                            content += `<div class="account-item">
                                <div><span class="rank-badge">#${index + 1}</span></div>
                                <div>${account.acct_num}</div>
                                <div>${account.last_name}</div>
                                <div>${account.first_name}</div>
                                <div class="balance-highlight">₹${account.balance.toFixed(2)}</div>
                            </div>`;
                        });
                    }
                }

                showResults(`🔄 ${sortTitle}`, content);
                showStatus(`✅ ${sortTitle} completed!`, 'success');
            } else {
                showResults('❌ Sort Failed', `<div style="text-align: center; padding: 20px; color: #dc3545;">Failed to sort: ${result.message}</div>`);
                showStatus(`❌ Sort failed: ${result.message}`, 'error');
            }
        }

        async function exportAccounts() {
            showLoading(true);
            const result = await apiCall('/api/accounts/export', 'POST');

            const content = `<div style="text-align: center; padding: 40px;">
                <div style="font-size: 48px; margin-bottom: 20px;">${result.success ? '✅' : '❌'}</div>
                <h3>${result.success ? 'Export Successful!' : 'Export Failed'}</h3>
                <p style="margin: 20px 0; color: #666;">${result.message}</p>
                ${result.success ? '<p style="color: #28a745;"><strong>File saved as: accounts.txt</strong></p>' : ''}
            </div>`;

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

            const content = `<div style="text-align: center; padding: 40px;">
                <div style="font-size: 48px; margin-bottom: 20px;">${result.success ? '✅' : '❌'}</div>
                <h3>${result.success ? 'Account Added Successfully!' : 'Failed to Add Account'}</h3>
                <p style="margin: 20px 0; color: #666;">${result.message}</p>
                ${result.success ? `<div style="background: #f8f9fa; padding: 15px; border-radius: 8px; margin: 20px 0;">
                    <strong>Account Details:</strong><br>
                    Account #: ${data.account_num}<br>
                    Name: ${data.first_name} ${data.last_name}<br>
                    Balance: ₹${parseFloat(data.balance).toFixed(2)}
                </div>` : ''}
            </div>`;

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

            const content = `<div style="text-align: center; padding: 40px;">
                <div style="font-size: 48px; margin-bottom: 20px;">${result.success ? '✅' : '❌'}</div>
                <h3>${result.success ? 'Account Updated Successfully!' : 'Failed to Update Account'}</h3>
                <p style="margin: 20px 0; color: #666;">${result.message}</p>
                ${result.success ? `<div style="background: #f8f9fa; padding: 15px; border-radius: 8px; margin: 20px 0;">
                    <strong>Transaction Details:</strong><br>
                    Account #: ${data.account_num}<br>
                    Transaction: ${parseFloat(data.transaction) >= 0 ? '+' : ''}₹${parseFloat(data.transaction).toFixed(2)}
                </div>` : ''}
            </div>`;

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

            const content = `<div style="text-align: center; padding: 40px;">
                <div style="font-size: 48px; margin-bottom: 20px;">${result.success ? '✅' : '❌'}</div>
                <h3>${result.success ? 'Account Deleted Successfully!' : 'Failed to Delete Account'}</h3>
                <p style="margin: 20px 0; color: #666;">${result.message}</p>
                ${result.success ? `<div style="background: #f8f9fa; padding: 15px; border-radius: 8px; margin: 20px 0;">
                    <strong>Deleted Account #: ${data.account_num}</strong>
                </div>` : ''}
            </div>`;

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
            setTimeout(() => statusElement.style.display = 'none', 5000);
        }
    </script>
</body>
</html>'''

# API Routes for V6 functionality

@app.route('/api/accounts', methods=['GET'])
def get_accounts():
    """Get all accounts with V6 validation"""
    try:
        accounts = bank.read_accounts_from_file()
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
    """Enhanced V6 sorting with all 4 options"""
    try:
        data = request.get_json()
        criterion = int(data['criterion'])  # 1=Balance, 2=Name, 3=Max, 4=Min
        order = int(data['order'])          # 1=Ascending, 2=Descending

        print(f"=== V6 SORT: Criterion {criterion}, Order {order} ===")

        result = bank.sort_accounts(criterion, order)

        if result['success']:
            lines = result['output'].split('\n')

            # Handle Max/Min Balance (criterion 3 or 4)
            if criterion == 3 or criterion == 4:
                for line in lines:
                    line = line.strip()
                    if line and not any(x in line for x in ['Enter your choice', 'Choose', 'Acct', '====', '?']):
                        parts = line.split()
                        if len(parts) >= 4:
                            try:
                                acct_num = int(parts[0])
                                if 1 <= acct_num <= 100:  # Valid account number
                                    last_name = parts[1]
                                    first_name = parts[2]
                                    balance = float(parts[3])

                                    return jsonify({
                                        "success": True,
                                        "message": f"{'Maximum' if criterion == 3 else 'Minimum'} balance account found",
                                        "account": {
                                            'acct_num': acct_num,
                                            'last_name': last_name,
                                            'first_name': first_name,
                                            'balance': balance
                                        }
                                    })
                            except (ValueError, IndexError):
                                continue

                return jsonify({"success": False, "message": "No valid account found for min/max"})

            # Handle sorted lists (criterion 1 or 2)
            else:
                sorted_accounts = []

                for line in lines:
                    line = line.strip()
                    if not line or any(x in line for x in ['Enter your choice', 'Choose', 'Acct', '====', '?']):
                        continue

                    parts = line.split()
                    if len(parts) >= 4:
                        try:
                            acct_num = int(parts[0])
                            if 1 <= acct_num <= 100:  # Valid account number
                                last_name = parts[1]
                                first_name = parts[2]
                                balance = float(parts[3])

                                sorted_accounts.append({
                                    'acct_num': acct_num,
                                    'last_name': last_name,
                                    'first_name': first_name,
                                    'balance': balance
                                })
                        except (ValueError, IndexError):
                            continue

                if sorted_accounts:
                    sort_type = "Balance" if criterion == 1 else "Name"
                    sort_order = "Ascending" if order == 1 else "Descending"

                    return jsonify({
                        "success": True,
                        "message": f"Sorted by {sort_type} ({sort_order})",
                        "accounts": sorted_accounts
                    })
                else:
                    # Fallback: read accounts and sort in Python
                    accounts = bank.read_accounts_from_file()
                    if accounts:
                        if criterion == 1:  # Sort by balance
                            sorted_accounts = sorted(accounts, key=lambda x: x['balance'], reverse=(order == 2))
                        else:  # Sort by name
                            sorted_accounts = sorted(accounts, key=lambda x: (x['last_name'], x['first_name']), reverse=(order == 2))

                        return jsonify({
                            "success": True,
                            "message": f"Sorted by {'Balance' if criterion == 1 else 'Name'} ({'Desc' if order == 2 else 'Asc'}) - Python fallback",
                            "accounts": sorted_accounts
                        })

                    return jsonify({"success": False, "message": "No accounts to sort"})

        else:
            return jsonify({"success": False, "message": f"Sort failed: {result['error']}"})

    except Exception as e:
        print(f"Sort error: {e}")
        return jsonify({"success": False, "message": str(e)})

@app.route('/api/test', methods=['GET'])
def test_c_program():
    """Test endpoint to verify V6 C program communication"""
    result = bank.call_c_program("6")  # Just test with exit command
    return jsonify({
        "c_program_working": result['success'],
        "output": result['output'],
        "error": result['error'],
        "version": "V6 Enhanced"
    })

if __name__ == '__main__':
    print("=== V6 Bank Account Manager with Enhanced C Backend ===")
    print("🚀 V6 Features: Advanced Sorting, Data Validation, Corruption Filtering")
    print("Test the connection: http://localhost:5000/api/test")
    print("Main interface: http://localhost:5000")
    print("========================================================")
    app.run(debug=True, host='0.0.0.0', port=5000)