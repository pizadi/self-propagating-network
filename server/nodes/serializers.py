from rest_framework import serializers
from .models import *

class NodeSerializer(serializers.Serializer):
    # node_id = serializers.IntegerField()
    class Meta:
        model = Node
        fields = ['node_id', 'time_connected', 'time_packet_send']
    
    def create(self, validated_data):
        return Node.objects.create(**validated_data)

class LightSensorSerializer(serializers.Serializer):
    class Meta:
        model = SensorLight
        fields = ['node_id', 'sensor_id', 'light']

class TemperatureSensorSerializer(serializers.Serializer):
    class Meta:
        model = SensorTemperature
        fields = ['node_id', 'sensor_id', 'temperature']