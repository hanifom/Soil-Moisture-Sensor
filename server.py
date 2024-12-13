from flask import Flask, request, jsonify

app = Flask(__name__)

import json

@app.route('/data')
def get_data():
    # Load data from your JSON source
    with open('data.json') as file:
        data = json.load(file)
    return data


if __name__ == '__main__':
    # Run the server on your local network
    app.run(host='0.0.0.0', port=5000)