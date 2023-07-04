from django.urls import path
from nodes.views import *

urlpatterns = [
    path('create/', NodeCreateView.as_view()),
    path('show/', SeeNodesView.as_view())
]