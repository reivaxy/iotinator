
console.log("FAN LOADED");
var fanUIClass = {
  // Model for the fan module
  Model: Backbone.Model.extend({
    defaults: function () {
      let model = {
        osc: "",  // "on" or "off"
        speed: 0
      };
      return model;
    }
  }),

  // View for the fan module
  View: XIOT.View.extend({
    tagName: "div",
    template: _.template(
      '<div class="btn-group" data-toggle="buttons">' +
        '<label class="btn btn-primary <%- speed==0?"active":"" %>" value="0">' +
        '<input type="radio" name="options" id="option0" autocomplete="off" <%- speed==0?"checked":"" %>>Off</label>' +
        '<label class="btn btn-primary <%- speed==1?"active":"" %>" value="1">' +
        '<input type="radio" name="options" id="option1" autocomplete="off" <%- speed==1?"checked":"" %>>Low</label>' +
        '<label class="btn btn-primary <%- speed==2?"active":"" %>" value="2">' +
        '<input type="radio" name="options" id="option2" autocomplete="off" <%- speed==2?"checked":"" %>>Med</label>' +
        '<label class="btn btn-primary <%- speed==3?"active":"" %>" value="3">' +
        '<input type="radio" name="options" id="option3" autocomplete="off" <%- speed==3?"checked":"" %>>High</label>' +
      '</div>' +
      '<button type="button" class="btn btn-primary <%- osc=="on"?"active":"" %>" data-toggle="button" aria-pressed="<%- osc=="on"?"true":"false" %>" autocomplete="off">Osc</button>'
      
    ),
    
    initialize: function () {
      this.listenTo(this.model, 'change', this.render);
    },
    events: {
      "click label.btn": "speed",
      "click button.btn": "oscillation",
    },
    speed: function(e) {
      let label = e.target;
      let value = parseInt(label.getAttribute("value"));
      this.model.set('speed', value);
      this.xiotSync(this.model);
    },
    oscillation: function(e) {
      let button = e.target;
      let previous = button.getAttribute("aria-pressed");
      if("true" === previous) {
        this.model.set('osc', "off");
      } else {
        this.model.set('osc', "on");
      }
      this.xiotSync(this.model);
    },
    render: function () {
      this.$el.html(this.template(this.model.toJSON()));
      return this;
    }
  })

  
};