from flask import Flask, render_template, jsonify
from influxdb_client import InfluxDBClient

app = Flask(__name__)

# InfluxDB Cloud credentials
INFLUX_URL = "https://us-east-1-1.aws.cloud2.influxdata.com"  # Replace with your region's URL
TOKEN = "E43_Y6JwUS-1bhvLv6Mk08bng4y8rJdRKP-FDXfY1vPQ8bzvwD0x7bQzjpGwE8FGSR8CM-ARSh3XPoqaN2FF5g=="  # Replace with your actual token
ORG = "iot-project"
BUCKET = "cloud_project"

client = InfluxDBClient(url=INFLUX_URL, token=TOKEN, org=ORG)
query_api = client.query_api()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/data")
def get_data():
    query = f'''
    from(bucket: "{BUCKET}")
      |> range(start: -1d)
      |> filter(fn: (r) => r._measurement == "water_level")
      |> filter(fn: (r) => r._field == "litres")
      |> keep(columns: ["_time", "_value"])
      |> sort(columns: ["_time"], desc: false)
    '''
    result = query_api.query(org=ORG, query=query)

    times, litres = [], []
    latest_litres = None

    for table in result:
        for record in table.records:
            times.append(record["_time"].strftime('%Y-%m-%d %H:%M:%S'))
            litres.append(record["_value"])

    if litres:
        latest_litres = litres[-1]  # Get the most recent litres value

    return jsonify({"time": times, "litres": litres, "latest_litres": latest_litres})

if __name__ == "__main__":
    app.run(debug=True)
