RSpec.describe Readapt::Message::Evaluate do
  it 'evaluates expressions' do
    bind = proc {
      value = 1
      send(:binding)
    }.call
    @debugger = double(:Debugger)
    @frame = Readapt::Frame.new(nil, 0, nil, bind.object_id)
    allow(@debugger).to receive(:frame) { @frame }
    arguments = {
      'expression' => '[value]'
    }
    message = Readapt::Message::Evaluate.new(arguments, @debugger)
    message.run
    result = message.body[:result]
    expect(result).to eq('[1]')
  end
end
