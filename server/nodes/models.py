from django.db import models

# Create your models here.
class Node(models.Model):
    id = models.AutoField(primary_key=True)
    node_id = models.IntegerField(null=False, blank=False)
    time_connected = models.DateTimeField(auto_created=True, blank=True, null=True)
    time_packet_send = models.DateField(auto_now=True, blank=True, null=True)

class SensorLight(models.Model):
    id = models.AutoField(primary_key=True)
    node_id = models.IntegerField(null=False, blank=False)
    sensor_id = models.IntegerField(null=False, blank=False)
    light = models.BooleanField()

class SensorTemperature(models.Model):
    id = models.AutoField(primary_key=True)
    node_id = models.IntegerField(null=False, blank=False)
    sensor_id = models.IntegerField(null=False, blank=False)
    temperature = models.FloatField(max_length=10)