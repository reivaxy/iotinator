
var switchUIClass = {
  // Model for the switch module
  Model: Backbone.Model.extend({
    defaults: function () {
      let model = {
        id: "",
        status: ""  // "on" or "off"
      };
      return model;
    }
  }),

  // View for the switch module
  View: Backbone.View.extend({
    tagName: "div",
    template: _.template('\
<div class="btn-group btn-group-toggle" data-toggle="buttons">\
  <label class="btn btn-secondary <%- status==\"on\"?\"active\":\"\" %>">\
    <input type="radio" name="options" id="optionOn" autocomplete="off" <%- status=="on"?"checked":"" %>> On\
  </label>\
  <label class="btn btn-secondary <%- status==\"off\"?\"active\":\"\" %>">\
    <input type="radio" name="options" id="optionOff" autocomplete="off" <%- status=="off"?"checked":"" %>> Off\
  </label>\
</div>'),
    
    initialize: function () {
      this.listenTo(this.model, 'change', this.render);
    },
    events: {
      "click label.btn": "toggle",
    },
    toggle: function(e) {
      if(this.model.attributes.status == "off") {
        this.model.attributes.status = "on";
      } else {
        this.model.attributes.status = "off";
      }
      Backbone.sync("update", this.model, {url:document.location.href + "api/data", headers:{"Xiot-forward-to": this.model.__ip}});
      // super crappy
      this.model.set(this.model.toJSON());
      this.render();
    },
    render: function () {
      this.$el.html(this.template(this.model.toJSON()));
      return this;
    }
  })

  
};