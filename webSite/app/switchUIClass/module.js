
var switchUIClass = {
  // Model for the switch module
  Model: Backbone.Model.extend({
    defaults: function () {
      let model = {
        status: ""  // "on" or "off"
      };
      return model;
    }
  }),

  // View for the switch module
  View: XIOT.View.extend({
    tagName: "div",
    template: _.template('\
<div class="btn-group btn-group-toggle" data-toggle="buttons">\
  <label class="btn btn-primary <%- status==\"off\"?\"active\":\"\" %>">\
    <input type="radio" name="options" id="optionOff" autocomplete="off" <%- status=="off"?"checked":"" %>> Off\
  </label>\
  <label class="btn btn-primary <%- status==\"on\"?\"active\":\"\" %>">\
    <input type="radio" name="options" id="optionOn" autocomplete="off" <%- status=="on"?"checked":"" %>> On\
  </label>\
</div>'),
    
    initialize: function () {
      this.listenTo(this.model, 'change', this.render);
    },
    events: {
      "click label.btn": "toggle",
    },
    toggle: function(e) {
      if(this.model.get("status") == "off") {
        this.model.set({status:"on"});
      } else {
        this.model.set({status:"off"});
      }
      this.xiotSync(this.model);
    },
    render: function () {
      this.$el.html(this.template(this.model.toJSON()));
      return this;
    }
  })

  
};