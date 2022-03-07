from django.urls import path
from . import views

urlpatterns = [
  #path("", views.home),
  path("cooluser/", views.CoolUserList.as_view(), name="cooluserlist"),
  path("design/<pk>", views.DesignList.as_view(), name="designlist"),
  path("", views.UserFormCreate.as_view(), name="createuser")

]