from msilib.schema import ListView
from unicodedata import name
from django.shortcuts import render
from django.template import loader
from django.http import HttpResponse
import random
from models import CoolUser, Design
from django.views.generic import ListView
from django.views.generic.edit import CreateView, UpdateView, DeleteView

# Create your views here.
def home(request):
    nickname = random.choice(nicknames)
    context = {"name" : nickname }
    return render(request, "coolflex/home.html", context)

nicknames = [
    "bestie", "girly", "queen", "king", "boo", "bud", "buddy", "bro", "broski", "bff", "soulmate", "stinky", "homegirl", "bruh"
]

class CoolUserList(ListView):
    model = CoolUser
    template = 'coolflex/home.html'

    def head(self, *args, **kwargs):
        user_list = self.get_queryset().order_by("first_name")
        response = HttpResponse(
            user_list
        )

class DesignList(ListView):
    model = Design
    template = 'coolflex/design.html'

    def head(self, *args, **kwargs):
        design_list = self.get_queryset()
        response = HttpResponse(
            design_list
        )