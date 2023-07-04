from rest_framework import status
from rest_framework.response import Response
from rest_framework.views import APIView
from .serializers import NodeSerializer
from rest_framework import generics
from .models import *
# from django.views.decorators.csrf import csrf_exempt
# Create your views here.

# @csrf_exempt
class NodeCreateView(APIView):
    """
    {
        "node_id": 12345,
        "time_connected": "2023-06-09T10:00:00Z",
        "time_packet_send": "2023-06-09"
    }

    """
    def post(self, request):
        serializer = NodeSerializer(data=request.data)
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data, status=status.HTTP_201_CREATED)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

class SeeNodesView(generics.ListAPIView):
    serializer_class = NodeSerializer
    queryset = Node.objects.all()