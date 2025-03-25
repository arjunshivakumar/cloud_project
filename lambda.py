import json
import boto3
from datetime import datetime
import logging

# Logging setup
logger = logging.getLogger()
logger.setLevel(logging.INFO)

# AWS Clients
s3 = boto3.client('s3')
sns_client = boto3.client('sns')

# S3 and SNS configuration
BUCKET_NAME = "water-level-data323"
SNS_TOPIC_ARN = "arn:aws:sns:eu-north-1:891076991562:WaterLevelAlerts"

# Water level thresholds
LOW_THRESHOLD = 1.0
HIGH_THRESHOLD = 80.0

def lambda_handler(event, context):
    logger.info("Received Event: %s", json.dumps(event))

    try:
        # Handle direct payload or IoT Core records
        if "Records" not in event:
            water_level = event.get("water_level")
            event_time = event.get("event_time")
            
            if water_level is not None and event_time is not None:
                store_as_csv(water_level, event_time, "ESP32_01")
                logger.info("Data stored successfully in S3.")
                return {
                    "statusCode": 200,
                    "body": json.dumps("Data stored in S3 and SNS alert triggered if needed.")
                }

        for record in event['Records']:
            payload = json.loads(record['body'])
            water_level = float(payload.get("water_level", 0.0))
            event_time = payload.get("event_time", datetime.utcnow().isoformat())
            device_id = payload.get("device_id", "ESP32_01")

            store_as_csv(water_level, event_time, device_id)

        logger.info("Data stored successfully.")
        return {
            "statusCode": 200,
            "body": json.dumps("Data stored in S3 and SNS alert triggered if needed.")
        }

    except Exception as e:
        logger.error(f"Error: {str(e)}")
        return {
            "statusCode": 500,
            "body": json.dumps("Failed to process event.")
        }


def store_as_csv(water_level, event_time, device_id):
    """
    Store water level data in S3 as CSV format and trigger SNS alerts if thresholds are breached.
    """
    timestamp = datetime.utcnow().strftime('%Y-%m-%dT%H:%M:%SZ')
    filename = f"{device_id}/{datetime.utcnow().strftime('%Y-%m-%d')}.csv"

    row = f"{timestamp},{device_id},{water_level},{event_time}\n"

    try:
        response = s3.get_object(Bucket=BUCKET_NAME, Key=filename)
        existing_data = response['Body'].read().decode('utf-8')
        new_data = existing_data + row
    except s3.exceptions.NoSuchKey:
        new_data = "timestamp,device_id,water_level,event_time\n" + row

    s3.put_object(Bucket=BUCKET_NAME, Key=filename, Body=new_data)
    logger.info(f"✅ Stored data in S3 (CSV): {filename}")

    if water_level < LOW_THRESHOLD or water_level > HIGH_THRESHOLD:
        alert_message = (
            f"🚨 ALERT: Water level = {water_level} cm crossed the threshold!\n"
            f"Device ID: {device_id}\n"
            f"Event Time: {event_time}"
        )

        sns_client.publish(TopicArn=SNS_TOPIC_ARN, Message=alert_message, Subject="Water Level Alert")
        logger.info(f"📢 SNS Alert Sent: {alert_message}")
    else:
        logger.info(f"✅ Water level is normal: {water_level} cm")
